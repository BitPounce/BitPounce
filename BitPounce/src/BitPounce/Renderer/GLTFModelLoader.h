#pragma once

#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <nlohmann/json.hpp>

#include "IModelLoader.h"

namespace BitPounce
{
    class GLTFModelLoader : public IModelLoader
    {
    public:
        bool CanLoad(const std::filesystem::path& filepath) const override;

        ModelLoadedData Load(const std::filesystem::path& filepath) override;

    private:
        std::filesystem::path m_Filepath;

        BufferBase m_Data;

        nlohmann::json m_JSON;

        ModelLoadedData m_LoadedData;

        std::vector<std::string> m_LoadedTextureNames;

        std::vector<std::pair<TextureType, Ref<Texture2D>>> m_LoadedTextures;

    private:
        void TraverseNode(uint32_t nodeIndex, glm::mat4 matrix = glm::mat4(1.0f));

        void LoadMesh(uint32_t meshIndex);

        BufferBase GetBinaryData();

        std::vector<float> GetFloats(const nlohmann::json& accessor);

        std::vector<uint32_t> GetIndices(const nlohmann::json& accessor);

        std::vector<std::pair<TextureType, Ref<Texture2D>>> GetTextures();

        std::vector<MeshVertex> AssembleVertices(
            const std::vector<glm::vec3>& positions,
            const std::vector<glm::vec3>& normals,
            const std::vector<glm::vec2>& texcoords
        );

        std::vector<glm::vec2> GroupVec2(const std::vector<float>& values);

        std::vector<glm::vec3> GroupVec3(const std::vector<float>& values);

        std::vector<glm::vec4> GroupVec4(const std::vector<float>& values);
    };
}