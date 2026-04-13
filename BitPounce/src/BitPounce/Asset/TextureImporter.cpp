#include "bp_pch.h"
#include "TextureImporter.h"

#include <stb_image.h>

namespace BitPounce {

	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		return ImportTexture2D(metadata.FilePath);
	}

	Ref<Texture2D> TextureImporter::ImportTexture2D(const std::filesystem::path& filepath)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = LoadTextureData(filepath, &width, &height, &channels);

		if (data.Data == nullptr)
		{
			BP_CORE_ERROR("TextureImporter::ImportTexture2D - Could not load texture from filepath: {}", filepath.string());
			return nullptr;
		}


		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;
		switch (channels)
		{
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec);
        texture->SetData(data.As<void>(), data.Size);
		data.Release();
		return texture;
	}

	Buffer TextureImporter::LoadTextureData(const std::filesystem::path& filepath, int* width, int* height, int* channels)
	{
		Buffer data;

		{
            BufferBase buffer = FileSystem::LoadFile(filepath);
			data.Data = stbi_load_from_memory(buffer.As<uint8_t>(), buffer.Size, width, height, channels, 0);

			// TODO: think about this
			data.Size = (*width) * (*height) * (*channels);
		}

		return data;
	}

}