#include "bp_pch.h"
#include "OpenGLShader.h"

#include "gl.h"
#include "BitPounce/Core/Base.h"
#include "BitPounce/Core/Logger.h"
#include <glm/gtc/type_ptr.hpp>
#include "BitPounce/Core/Buffer.h"
#include "BitPounce/Core/FileSystem.h"
#include <stb_include.h>

// I am soooooooo lazy 🦥
#define ShaderTypeHasNoBeenImplementedYet(type) BP_CORE_ASSERT(false, "Shader Type: {} Not Yet Implemented!", type)

namespace BitPounce {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		if (type == "geometry")
			ShaderTypeHasNoBeenImplementedYet("Geometry");
		if (type == "tessellation")
			ShaderTypeHasNoBeenImplementedYet("Tessellation");
		if (type == "compute")
			ShaderTypeHasNoBeenImplementedYet("Compute");

		BP_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		m_Name = name;

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		Compile(sources);
	}

	OpenGLShader::OpenGLShader(const std::filesystem::path& filepath)
	{
		BufferBase source = FileSystem::LoadFile(filepath);
		Buffer tmp = Buffer(source.Size);
		DiskMemCpy(tmp.As<uint8_t>(), source.Data, source.Size);
		std::string src(tmp.As<char>(), tmp.Size);
		auto shaderSources = PreProcess(src, filepath);
		Compile(shaderSources);
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source, const std::filesystem::path& filepath)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		// Step 1: Split source by #type
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			BP_CORE_ASSERT(eol != std::string::npos, "Syntax error: Missing end-of-line after #type");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			BP_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			shaderSources[ShaderTypeFromString(type)] = source.substr(
				nextLinePos,
				pos - (nextLinePos == std::string::npos ? source.size() : nextLinePos)
			);
		}

		// Step 2: Prepend #version only
		for (auto& src : shaderSources)
		{
			std::string header = "#version 300 es\nprecision mediump float;\n";
			src.second = header + src.second;
		}

		// Step 3: Handle includes
		std::string includePath = filepath.parent_path().string();
		std::string filename = filepath.filename().string();
		std::string filepathStr = includePath + '/' + filename;

		auto lastSlash = filepathStr.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepathStr.rfind('.');
		auto count = lastDot == std::string::npos ? filepathStr.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepathStr.substr(lastSlash, count);

		for (auto& src : shaderSources)
		{
			char error[256];
			char* result = stb_include_string(const_cast<char*>(src.second.c_str()), nullptr,
				const_cast<char*>(includePath.c_str()), const_cast<char*>(filename.c_str()), error);

			BP_CORE_ASSERT(result, "stb_include_string failed: {}", error);

			std::string fixedResult = result;
    free(result);

    size_t linePos = 0;
    while ((linePos = fixedResult.find("#line")) != std::string::npos)
    {
        size_t endLine = fixedResult.find_first_of("\r\n", linePos);
        if (endLine == std::string::npos)
            endLine = fixedResult.size();
        fixedResult.erase(linePos, endLine - linePos);
    }

    src.second = fixedResult;
		}

		

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		std::vector<GLuint> glShaderIDs;

		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog.data());

				glDeleteShader(shader);
				BP_CORE_ERROR("{0}", infoLog.data());
				BP_CORE_ASSERT(false, "Shader compilation failure!");
				return;
			}

			glAttachShader(program, shader);
			glShaderIDs.push_back(shader);
		}

		m_RendererID = program;

		// Link program
		glLinkProgram(program);
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			glDeleteProgram(program);
			for (auto id : glShaderIDs)
				glDeleteShader(id);

			BP_CORE_ERROR("{0}", infoLog.data());
			BP_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// Detach shaders
		for (auto id : glShaderIDs)
			glDetachShader(program, id);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformInts(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	// Shortcut setters
	void OpenGLShader::SetInt(const std::string& name, int value) { UploadUniformInt(name, value); }
	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) { UploadUniformInts(name, values, count); }
	void OpenGLShader::SetFloat(const std::string& name, const float& value) { UploadUniformFloat(name, value); }
	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value) { UploadUniformFloat2(name, value); }
	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) { UploadUniformFloat3(name, value); }
	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) { UploadUniformFloat4(name, value); }
	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value) { UploadUniformMat3(name, value); }
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) { UploadUniformMat4(name, value); }

} // namespace BitPounce