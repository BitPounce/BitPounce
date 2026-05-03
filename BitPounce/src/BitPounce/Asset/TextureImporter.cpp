#include "bp_pch.h"
#include "TextureImporter.h"
#include "AssetManager.h"

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

	bool TextureImporter::ExportAsset(AssetHandle handle, const AssetMetadata &metadata, BitPouncePack::Pack &pack)
	{
		Ref<Texture2D> asset = AssetManager::GetAsset<Texture2D>(handle);
		BitPouncePack::PackTexture packTexture = BitPouncePack::PackTexture();
		packTexture.AssetHandle = handle;
		auto src = asset->GetData();
		size_t size = asset->GetWidth() * asset->GetHeight() * (uint8_t)asset->GetSpecification().Format;

		packTexture.Data = std::make_unique<std::byte[]>(size);
		std::memcpy(packTexture.Data.get(), src.Data, size);
		packTexture.Size = size;
		packTexture.Width = asset->GetWidth();
		packTexture.Height = asset->GetHeight();
		packTexture.Filter = (uint8_t)asset->GetSpecification().Filter;
		packTexture.Channels = (uint8_t)asset->GetSpecification().Format;
		BitPouncePack::Buffer bufffer = BitPouncePack::Buffer(size);
		memcpy(bufffer.As<void>(), asset->GetData().As<void>(), size);
		packTexture.hash = BitPouncePack::sha256(bufffer);
		pack.assets.push_back(BitPouncePack::PackAsset{ std::move(packTexture) });
		return true;
	}
	Ref<Texture2D> TextureImporter::LoadAsset(BitPouncePack::PackAsset &packAsset)
	{
		const auto t = std::get_if<BitPouncePack::PackTexture>(&packAsset);
		
		TextureSpecification spec;
		spec.Width = t->Width;
		spec.Height = t->Height;
		switch (t->Channels)
		{
			case 2:
				spec.Format = ImageFormat::RG8;
				break;
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData(t->Data.get(), t->Size);
		texture->Handle = t->AssetHandle;
		return texture;
	
	}
}