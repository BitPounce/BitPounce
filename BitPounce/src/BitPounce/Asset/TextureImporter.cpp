#include "bp_pch.h"
#include "TextureImporter.h"

#include <stb_image.h>

namespace BitPounce {

	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;

		{
			std::string pathStr = metadata.FilePath.string();
            BufferBase buffer = FileSystem::LoadFile(pathStr);
			data.Data = stbi_load_from_memory(buffer.As<uint8_t>(), buffer.Size, &width, &height, &channels, 0);
		}

		if (data.Data == nullptr)
		{
			BP_CORE_ERROR("TextureImporter::ImportTexture2D - Could not load texture from filepath: {}", metadata.FilePath.string());
			return nullptr;
		}

		// TODO: think about this
		data.Size = width * height * channels;

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

}