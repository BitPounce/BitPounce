#pragma once

#include "BitPounce/Renderer/Shader.h"
#include <glm/glm.hpp>
#include <filesystem>
#include <unordered_map>

// TODO: REMOVE!
typedef unsigned int GLenum;
namespace BitPounce
{
    class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::filesystem::path& filepath);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		void UploadUniformInt(const std::string& name, int value);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source, const std::filesystem::path& filepath);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
	};
}