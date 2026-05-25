#pragma once
#include "Shader.h"


namespace BitPounce
{
    template<typename T>
    struct MaterialPropArray
    {
        uint32_t count;
        T* values;
    };

    class Material
    {
    public:
        Material(Ref<Shader> shader);
        ~Material();
        void Bind()
        {
            m_Shader->Bind();
        }
        void UnBind()
        {
            m_Shader->Unbind();
        }

        static Ref<Material> Create(Ref<Shader> shader);
        Ref<Shader> GetShader() { return m_Shader; }

        void SetInt(const std::string& name, int value);
        void SetIntArray(const std::string& name, MaterialPropArray<int> ints);

        void SetFloat(const std::string& name, const float& value);
		void SetFloat2(const std::string& name, const glm::vec2& value);
		void SetFloat3(const std::string& name, const glm::vec3& value);
		void SetFloat4(const std::string& name, const glm::vec4& value);

        void SetMat3(const std::string& name, const glm::mat3& value);
        void SetMat4(const std::string& name, const glm::mat4& value);

        int GetInt(const std::string& name);
        MaterialPropArray<int> GetIntArray(const std::string& name);

        float GetFloat(const std::string& name);
		glm::vec2 GetFloat2(const std::string& name);
		glm::vec3 GetFloat3(const std::string& name);
		glm::vec4 GetFloat4(const std::string& name);

        glm::mat3 GetMat3(const std::string& name);
        glm::mat4 GetMat4(const std::string& name);
    
    private:
        Ref<Shader> m_Shader;
        std::unordered_map<std::string, std::any> m_Props;
    };
    
    
}