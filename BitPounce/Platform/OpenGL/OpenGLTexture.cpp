#include "bp_pch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include "gl.h"
#include "BitPounce/Core/Buffer.h"
#include "BitPounce/Core/FileSystem.h"

namespace BitPounce {

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

		glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, imageData);

		stbi_image_free(imageData);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

}
