#include "bp_pch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include "gl.h"
#include "BitPounce/Core/Buffer.h"
#include "BitPounce/Core/FileSystem.h"
#include "stb_image_write.h"

namespace BitPounce 
{
	namespace Utils { 

		static GLenum BitPounceImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:  return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
			}

			BP_CORE_ASSERT(false, "Formart haz no rizz");
			return 0;
		}
		
		static GLenum BitPounceImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:  return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
			}

			BP_CORE_ASSERT(false, "Formart haz no rizz");
			return 0;
		}

	}
	
	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		m_DataFormat = Utils::BitPounceImageFormatToGLDataFormat(m_Specification.Format);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if(specification.Filter == ImageFilter::LINEAR)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if(specification.Filter == ImageFilter::NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}


	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		m_DataFormat = GL_RGBA;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		BufferBase fileBuffer = FileSystem::LoadFile(path);

		constexpr uint64_t MaxRamSize = 1024ull * 1024ull * 1024ull; // 1 GB

		stbi_uc* imageData = nullptr;
		int width = 0, height = 0, channels = 0;

		stbi_set_flip_vertically_on_load(1);

		imageData = stbi_load_from_memory(fileBuffer.As<uint8_t>(), fileBuffer.Size, &width, &height, &channels, 0);

		BP_CORE_ASSERT(imageData, "Failed to load image!");

		m_Width = width;
		m_Height = height;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		GLenum format = GL_RGB;
		if (channels == 1) format = GL_RED;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		m_DataFormat = format;

		glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, imageData);

		stbi_image_free(imageData);
	}

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &specification, const std::string &path)
		: m_Path(path)
    {
		BufferBase fileBuffer = FileSystem::LoadFile(path);

		constexpr uint64_t MaxRamSize = 1024ull * 1024ull * 1024ull; // 1 GB

		stbi_uc* imageData = nullptr;
		int width = 0, height = 0, channels = 0;

		stbi_set_flip_vertically_on_load(1);

		imageData = stbi_load_from_memory(fileBuffer.As<uint8_t>(), fileBuffer.Size, &width, &height, &channels, 0);

		BP_CORE_ASSERT(imageData, "Failed to load image!");

		m_Width = width;
		m_Height = height;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if(specification.Filter == ImageFilter::LINEAR)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if(specification.Filter == ImageFilter::NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		GLenum format = GL_RGB;
		if (channels == 1) format = GL_RED;
		else if (channels == 3) format = GL_RGB;
		else if (channels == 4) format = GL_RGBA;

		m_DataFormat = format;

		glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, imageData);

		stbi_image_free(imageData);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::ToPNG(std::filesystem::path filepath)
	{
		int channels = -1;
		if (m_DataFormat == GL_RED) channels = 1;
		else if (m_DataFormat == GL_RGB) channels = 3;
		else if (m_DataFormat == GL_RGBA) channels = 4;

		Bind();
		int data_size = m_Width * m_Height * channels;
		GLubyte* pixels = new GLubyte[data_size];

		stbi_flip_vertically_on_write(1);

		{
			// HACK: ES does not support glGetTexImage, whyyyyyyyy!!!
			GLuint fbo;
			glGenFramebuffers(1, &fbo); 
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RendererID, 0);

			glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &fbo);
		}

		stbi_write_png(filepath.generic_string().c_str(), m_Width, m_Height, channels, pixels, m_Width * channels);

		delete[] pixels;
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		BP_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTexImage2D(GL_TEXTURE_2D, 0, m_DataFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

}
