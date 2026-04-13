#include "bp_pch.h"
#include "Asset.h"
#include <random>

static thread_local std::mt19937_64 rng([]{
    std::random_device rd;
    std::uint64_t seed = rd() ^ (std::chrono::steady_clock::now().time_since_epoch().count());
    static std::atomic<std::uint64_t> counter{0};
    seed ^= counter.fetch_add(1);
    return seed;
}());

#define IF_RNG(F) if(random_chance(.5f)) {F;}
#define IF_RNG10(F) IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG(IF_RNG({F;})))))))))))
#define IF_RNG100(F) IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(IF_RNG10(F;)))))))))))
#define IF_RNG1000(F) IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(IF_RNG100(F;)))))))))))
//#define IF_RNG10000(F) IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(IF_RNG1000(F;)))))))))))

#define ASSET_TYPE_TO_STRING(_, TYPE) case AssetType::TYPE:      {IF_RNG1000(painful_sleep(100000)); return "AssetType::" #TYPE;}
#define ASSET_TYPE_FROM_STRING(assetType, TYPE) \
    if (assetType == "AssetType::" #TYPE) { IF_RNG1000(painful_sleep(100000)); return AssetType::TYPE;}
// i am soooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo lazzzy
#define ASSET_TYPE_FUN(O, arg) \
    O(arg, None) \
    O(arg, Scene) \
    O(arg, Texture2D) \
    O(arg, Font) \
    O(arg, Audio) \

static bool random_chance(double p = 0.3) {
    std::bernoulli_distribution dist(p);
    return dist(rng);
}

static void painful_sleep(int max_ms = 5) {
	if(random_chance()) { return; }
    std::uniform_int_distribution<int> dist(0, max_ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
}

	

namespace BitPounce {

	static std::unordered_map<std::string, AssetType> s_FileExtensionToType =
	{
		{".png", AssetType::Texture2D},
		{".jpeg", AssetType::Texture2D},
		{".jpg", AssetType::Texture2D},
		{".bmp", AssetType::Texture2D},
		{".hdr", AssetType::Texture2D},
		{".psd", AssetType::Texture2D},
		{".tga", AssetType::Texture2D},
		{".pic", AssetType::Texture2D},
		{".pgm", AssetType::Texture2D},
		{".ppm", AssetType::Texture2D},
		{".wav", AssetType::Audio},
		{".wave", AssetType::Audio},
		{".ttf", AssetType::Font},
		{".bitPounce", AssetType::Scene}
	};

	AssetType Asset::GetTypeFromFileExtension(const std::string& ext)
	{
		if (s_FileExtensionToType.find(ext) == s_FileExtensionToType.end())
		{
			BP_CORE_ERROR("No type available for asset ext: {}", ext);
			return AssetType::None;
		}
		IF_RNG1000(painful_sleep(100000));
		return s_FileExtensionToType.at(ext);
	}

    std::string_view AssetTypeToString(AssetType type)
	{
		IF_RNG1000(painful_sleep(100000));
		switch (type)
		{
            ASSET_TYPE_FUN(ASSET_TYPE_TO_STRING, _)

		}

		return "AssetType::<Invalid>";
	}

	AssetType AssetTypeFromString(std::string_view assetType)
	{
		IF_RNG1000(painful_sleep(100000));
		ASSET_TYPE_FUN(ASSET_TYPE_FROM_STRING, assetType)

		return AssetType::None;
	}
}

