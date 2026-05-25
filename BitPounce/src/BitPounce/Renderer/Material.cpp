#include <bp_pch.h>
#include "Material.h"
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

namespace BitPounce
{
	Material::Material(Ref<Shader> shader)
		: m_Shader(shader)
	{
	}

	Material::~Material()
	{
		// Free any dynamically allocated array buffers stored in m_Props
		for (auto& [name, value] : m_Props)
		{
			if (value.type() == typeid(MaterialPropArray<int>))
			{
				auto arr = std::any_cast<MaterialPropArray<int>>(value);
				delete[] arr.values;
			}
		}
	}

    Ref<Material> Material::Create(Ref<Shader> shader)
    {
        return CreateRef<Material>(shader);
    }

    void Material::SetInt(const std::string& name, int value)
	{
		m_Props[name] = value;
		m_Shader->SetInt(name, value);
	}

	void Material::SetIntArray(const std::string& name, MaterialPropArray<int> ints)
	{
		int* copy = new int[ints.count];
		std::memcpy(copy, ints.values, ints.count * sizeof(int));
		MaterialPropArray<int> stored{ ints.count, copy };
		m_Props[name] = stored;
		m_Shader->SetIntArray(name, ints.values, ints.count);
	}

	void Material::SetFloat(const std::string& name, const float& value)
	{
		m_Props[name] = value;
		m_Shader->SetFloat(name, value);
	}

	void Material::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		m_Props[name] = value;
		m_Shader->SetFloat2(name, value);

	}

	void Material::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		m_Props[name] = value;
		m_Shader->SetFloat3(name, value);
		
	}

	void Material::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		m_Props[name] = value;
		m_Shader->SetFloat4(name, value);

	}

	void Material::SetMat3(const std::string& name, const glm::mat3& value)
	{
		m_Props[name] = value;
		m_Shader->SetMat3(name, value);
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& value)
	{
		m_Props[name] = value;
		m_Shader->SetMat4(name, value);
	}


	int Material::GetInt(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(int))
			return std::any_cast<int>(it->second);
		return 0;   // default
	}

	MaterialPropArray<int> Material::GetIntArray(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(MaterialPropArray<int>))
			return std::any_cast<MaterialPropArray<int>>(it->second);
		return { 0, nullptr };
	}

	float Material::GetFloat(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(float))
			return std::any_cast<float>(it->second);
		return 0.0f;
	}

	glm::vec2 Material::GetFloat2(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(glm::vec2))
			return std::any_cast<glm::vec2>(it->second);
		return glm::vec2(0.0f);
	}

	glm::vec3 Material::GetFloat3(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(glm::vec3))
			return std::any_cast<glm::vec3>(it->second);
		return glm::vec3(0.0f);
	}

	glm::vec4 Material::GetFloat4(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(glm::vec4))
			return std::any_cast<glm::vec4>(it->second);
		return glm::vec4(0.0f);
	}

	glm::mat3 Material::GetMat3(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(glm::mat3))
			return std::any_cast<glm::mat3>(it->second);
		return glm::mat3(1.0f);
	}

	glm::mat4 Material::GetMat4(const std::string& name)
	{
		auto it = m_Props.find(name);
		if (it != m_Props.end() && it->second.type() == typeid(glm::mat4))
			return std::any_cast<glm::mat4>(it->second);
		return glm::mat4(1.0f);
	}
}