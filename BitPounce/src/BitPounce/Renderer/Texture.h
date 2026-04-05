#pragma once

#include <string>

#include "BitPounce/Core/Base.h"

namespace BitPounce 
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	enum class ImageFilter
	{
		NEAREST = 0,
		LINEAR
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		ImageFilter Filter = ImageFilter::NEAREST;
		bool GenerateMips = true;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual const std::string& GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification);
		// DO NOT USE
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);

		virtual void ToPNG(std::filesystem::path filepath) = 0;

		virtual bool operator==(const Texture2D& other) const = 0;
	};

}