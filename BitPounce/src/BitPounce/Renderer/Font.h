#pragma once
#include <filesystem>
#include "BitPounce/Renderer/Texture.h"
#include "BitPounce/Asset/Asset.h"
#include <thread>

namespace msdf_atlas
{
	class FontGeometry;
	class GlyphGeometry;
}

namespace BitPounce 
{
    struct MSDFData;

	class Font: public Asset
	{
	public:
		Font(const std::filesystem::path& font);
        ~Font();

        const MSDFData* GetMSDFData() const { return m_Data; }
        Ref<Texture2D> GetAtlasTexture() const;
        static Ref<Font> GetDefault();
		virtual AssetType GetType() const
		{
			return AssetType::Font;
		}
    private:
		MSDFData* m_Data;
		mutable Ref<Texture2D> m_AtlasTexture;
		mutable bool m_IsTextureMaked = false;
		mutable bool m_IsTextureLoaded = false;
		mutable std::thread m_LoadingThread;
		Buffer* m_FileBuffer = 0;
	};

}