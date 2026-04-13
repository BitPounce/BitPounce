#include "bp_pch.h"
#include "Font.h"
#include <thread>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cmath>   // for std::isinf
#include <BitPounce/Core/Timer.h>

#undef INFINITE
#include "msdf-atlas-gen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"
#include "Rectangle.h"

#include "MSDFData.h"
#include "BitPounce/Project/Project.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace BitPounce {

	// ------------------------------------------------------------------------
	// Cache file path helpers
	// ------------------------------------------------------------------------
	static std::string GetCacheBasePath(const std::filesystem::path& fontPath, double emSize)
	{
		auto fileSize = std::filesystem::file_size(fontPath);
		auto ftime = std::filesystem::last_write_time(fontPath);
		auto timeStr = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
			ftime.time_since_epoch()).count());

		std::string combined = fontPath.string() + "_" + std::to_string(fileSize) + "_" + timeStr + "_" + std::to_string(emSize);
		std::hash<std::string> hasher;
		size_t hash = hasher(combined);
		std::string baseName = "font_cache_" + std::to_string(hash);

		std::filesystem::path cacheDir = Project::GetAssetFileSystemPath("Cache");
		if (!std::filesystem::exists(cacheDir))
			std::filesystem::create_directories(cacheDir);

		return (cacheDir / baseName).string();
	}

	static std::string GetCacheJsonPath(const std::filesystem::path& fontPath, double emSize)
	{
		return GetCacheBasePath(fontPath, emSize) + ".json";
	}

	static std::string GetCachePngPath(const std::filesystem::path& fontPath, double emSize)
	{
		return GetCacheBasePath(fontPath, emSize) + ".png";
	}

	// ------------------------------------------------------------------------
	// Save to cache: PNG + JSON (atlas bounds using getQuadAtlasBounds)
	// ------------------------------------------------------------------------
	static void SaveToCache(const std::filesystem::path& fontPath,
							double emSize,
							const Ref<Texture2D>& texture,
							const std::vector<msdf_atlas::GlyphGeometry>& glyphs)
	{
		std::string pngPath = GetCachePngPath(fontPath, emSize);
		texture->ToPNG(pngPath);   // save texture as PNG

		json j;
		j["version"] = 1;
		j["png_file"] = std::filesystem::path(pngPath).filename().string();
		j["em_size"] = emSize;
		j["pixel_range"] = 4.0;      // must match generation
		j["miter_limit"] = 1.0;      // must match generation

		json glyphsArray = json::array();
		for (const auto& glyph : glyphs) {
			double l, b, r, t;
			glyph.getQuadAtlasBounds(l, b, r, t);   // public getter
			glyphsArray.push_back({ l, b, r, t });
		}
		j["atlas_bounds"] = glyphsArray;

		std::string jsonPath = GetCacheJsonPath(fontPath, emSize);
		std::ofstream file(jsonPath);
		if (file) {
			file << j.dump(4);
			BP_CORE_INFO("Saved font atlas to cache: {}", jsonPath);
		} else {
			BP_CORE_WARN("Could not write cache file: {}", jsonPath);
		}
	}

	// ------------------------------------------------------------------------
	// Load from cache: load PNG, reload font, wrap glyphs, then set atlas bounds
	// ------------------------------------------------------------------------
	static bool LoadFromCache(const std::filesystem::path& fontPath,
							  double emSize,
							  Ref<Texture2D>& outTexture,
							  MSDFData* outData)
	{
		std::string jsonPath = GetCacheJsonPath(fontPath, emSize);
		std::ifstream file(jsonPath);
		if (!file) return false;

		json j;
		try { file >> j; }
		catch (...) { return false; }

		if (j.value("version", 0) != 1) return false;

		std::string pngFilename = j["png_file"];
		std::filesystem::path pngFullPath = std::filesystem::path(jsonPath).parent_path() / pngFilename;
		if (!std::filesystem::exists(pngFullPath)) return false;

		Timer timer = Timer();
		// Load texture from PNG (requires Texture2D::Create(const std::string&))
		outTexture = Texture2D::Create(pngFullPath.string());
		if (!outTexture) return false;
		BP_CORE_INFO("{0}", timer.Elapsed());

		// ------------------------------------------------------------
		// Reload the font (fast) to get glyphs with correct advances & shape
		// ------------------------------------------------------------
		timer.Reset();
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft) return false;

		BufferBase buffer = FileSystem::ReadFileBinary(fontPath);
		timer.Reset();
		BP_CORE_INFO("{0}", timer.Elapsed());
		msdfgen::FontHandle* font = msdfgen::loadFontData(ft, buffer.As<msdfgen::byte>(), buffer.Size);
		BP_CORE_INFO("{0}", timer.Elapsed());
		if (!font) {
			msdfgen::deinitializeFreetype(ft);
			return false;
		}

		struct CharsetRange { uint32_t Begin, End; };
		static const CharsetRange charsetRanges[] = {
			{ 0x0020, 0x007E }, { 0x00A0, 0x00FF },
			{ 0x0100, 0x017F }, { 0x0180, 0x024F }
		};
		msdf_atlas::Charset charset;
		BP_CORE_INFO("{0}", timer.Elapsed());
		for (auto& range : charsetRanges)
			for (uint32_t c = range.Begin; c <= range.End; ++c)
				charset.add(c);
		BP_CORE_INFO("{0}", timer.Elapsed());

		double fontScale = 2.0;
		outData->Glyphs.clear();
		outData->FontGeometry = msdf_atlas::FontGeometry(&outData->Glyphs);
		int glyphsLoaded = outData->FontGeometry.loadCharset(font, fontScale, charset);
		if (glyphsLoaded == 0) {
			msdfgen::destroyFont(font);
			msdfgen::deinitializeFreetype(ft);
			return false;
		}
		BP_CORE_INFO("{0}", timer.Elapsed());

		// ------------------------------------------------------------
		// Compute plane bounds (same as TightAtlasPacker would do)
		// ------------------------------------------------------------
		double pixelRange = j.value("pixel_range", 4.0);
		double miterLimit = j.value("miter_limit", 1.0);
		double scale = j.value("em_size", emSize);
		
		for (auto& glyph : outData->Glyphs) {
			glyph.wrapBox(scale, pixelRange, miterLimit, false, false); // px align false
		}

		// ------------------------------------------------------------
		// Override atlas bounds with cached values
		// ------------------------------------------------------------
		const auto& cachedBounds = j["atlas_bounds"];
		if (outData->Glyphs.size() != cachedBounds.size()) {
			msdfgen::destroyFont(font);
			msdfgen::deinitializeFreetype(ft);
			return false;
		}

		for (size_t i = 0; i < outData->Glyphs.size(); ++i) {
			auto bounds = cachedBounds[i];
			double l = bounds[0], b = bounds[1], r = bounds[2], t = bounds[3];
			double w = r - l;
			double h = t - b;
			msdf_atlas::Rectangle rect(l, b, w, h);
			outData->Glyphs[i].setBoxRect(rect);
		}

		// Sanity check: ensure plane bounds are valid
		for (size_t i = 0; i < outData->Glyphs.size(); ++i) {
			double l, b, r, t;
			outData->Glyphs[i].getQuadPlaneBounds(l, b, r, t);
			if (std::isinf(l) || std::isinf(b) || std::isinf(r) || std::isinf(t)) {
				BP_CORE_ERROR("Glyph {} has invalid plane bounds after cache load", i);
				msdfgen::destroyFont(font);
				msdfgen::deinitializeFreetype(ft);
				return false;
			}
		}

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		BP_CORE_INFO("Loaded font atlas from cache: {}", jsonPath);
		return true;
	}

	// ------------------------------------------------------------------------
	// Atlas generation template (unchanged)
	// ------------------------------------------------------------------------
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize,
		const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		auto processors = std::thread::hardware_concurrency();
		if (processors == 0) processors = 1;

		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(processors);
		generator.generate(glyphs.data(), (int)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = (N == 4) ? ImageFormat::RGBA8 : ImageFormat::RGB8;
		spec.Filter = ImageFilter::LINEAR;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * N);
		return texture;
	}

	// ------------------------------------------------------------------------
	// Font constructor
	// ------------------------------------------------------------------------
	Font::Font(const std::filesystem::path& filepath)
		: m_Data(new MSDFData())
	{
		const double emSize = 160.0;

		// Try to load from cache
		if (LoadFromCache(filepath, emSize, m_AtlasTexture, m_Data))
			return;

		BP_CORE_INFO("Cache miss for font {}. Generating new atlas...", filepath.string());

		// ------------------------------------------------------------
		// Full generation (original code)
		// ------------------------------------------------------------
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		BP_CORE_ASSERT(ft, "Freetype is not working!");

		BufferBase buffer = FileSystem::LoadFile(filepath);
		msdfgen::FontHandle* font = msdfgen::loadFontData(ft, buffer.As<msdfgen::byte>(), buffer.Size);
		if (!font) {
			BP_CORE_ERROR("Failed to load font: {}", filepath.string());
			return;
		}

		struct CharsetRange { uint32_t Begin, End; };
		static const CharsetRange charsetRanges[] = {
			{ 0x0020, 0x007E }, { 0x00A0, 0x00FF },
			{ 0x0100, 0x017F }, { 0x0180, 0x024F }
		};

		msdf_atlas::Charset charset;
		for (auto& range : charsetRanges)
			for (uint32_t c = range.Begin; c <= range.End; ++c)
				charset.add(c);

		double fontScale = 2.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		BP_CORE_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(4.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setScale(emSize);
		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());

		int width, height;
		atlasPacker.getDimensions(width, height);
		double actualEmSize = atlasPacker.getScale();

		auto processors = std::thread::hardware_concurrency();
		if (processors == 0) processors = 1;

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT processors

		uint64_t coloringSeed = 0;
		bool expensiveColoring = true;
		if (expensiveColoring) {
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
			}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		} else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs) {
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 4, msdf_atlas::mtsdfGenerator>(
			"Test", (float)actualEmSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

		// Save to cache (PNG + JSON)
		SaveToCache(filepath, emSize, m_AtlasTexture, m_Data->Glyphs);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font()
	{
		delete m_Data;
	}

	Ref<Texture2D> Font::GetAtlasTexture() const
	{
		return m_AtlasTexture;
	}

	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<Font>("assets/fonts/OpenSans/static/OpenSans-Regular.ttf");
		
		return DefaultFont;
	}



}