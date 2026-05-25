#include <bp_pch.h>
#include "GLTFModelLoader.h"

namespace BitPounce
{
    bool GLTFModelLoader::CanLoad(const std::filesystem::path& filepath) const
    {
        return filepath.extension() == ".gltf";
    }

    ModelLoadedData GLTFModelLoader::Load(const std::filesystem::path& filepath)
    {
        m_Filepath = filepath;

        BufferBase jsonBuffer = FileSystem::LoadFile(filepath);

        std::string jsonText(
            reinterpret_cast<char*>(jsonBuffer.Data),
            jsonBuffer.Size
        );

        m_JSON = nlohmann::json::parse(jsonText);

        m_Data = GetBinaryData();

        TraverseNode(0);

        return m_LoadedData;
    }

    void GLTFModelLoader::TraverseNode(uint32_t nodeIndex, glm::mat4 matrix)
    {
        nlohmann::json node = m_JSON["nodes"][nodeIndex];

        glm::vec3 translation(0.0f);
        glm::vec3 scale(1.0f);
        glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
        glm::mat4 localMatrix(1.0f);

        if (node.contains("translation"))
        {
            float values[3];

            for (uint32_t i = 0; i < 3; i++)
                values[i] = node["translation"][i];

            translation = glm::make_vec3(values);
        }

        if (node.contains("scale"))
        {
            float values[3];

            for (uint32_t i = 0; i < 3; i++)
                values[i] = node["scale"][i];

            scale = glm::make_vec3(values);
        }

        if (node.contains("rotation"))
        {
            float values[4] =
            {
                node["rotation"][3],
                node["rotation"][0],
                node["rotation"][1],
                node["rotation"][2]
            };

            rotation = glm::make_quat(values);
        }

        if (node.contains("matrix"))
        {
            float values[16];

            for (uint32_t i = 0; i < 16; i++)
                values[i] = node["matrix"][i];

            localMatrix = glm::make_mat4(values);
        }

        glm::mat4 transform =
            matrix *
            localMatrix *
            glm::translate(glm::mat4(1.0f), translation) *
            glm::mat4_cast(rotation) *
            glm::scale(glm::mat4(1.0f), scale);

        if (node.contains("mesh"))
        {
            m_LoadedData.Translations.push_back(translation);
            m_LoadedData.Rotations.push_back(rotation);
            m_LoadedData.Scales.push_back(scale);
            m_LoadedData.Matrices.push_back(transform);

            LoadMesh(node["mesh"]);
        }

        if (node.contains("children"))
        {
            for (auto& child : node["children"])
                TraverseNode(child, transform);
        }
    }

    void GLTFModelLoader::LoadMesh(uint32_t meshIndex)
    {
        auto primitive = m_JSON["meshes"][meshIndex]["primitives"][0];

        uint32_t posAccessor = primitive["attributes"]["POSITION"];
        uint32_t normalAccessor = primitive["attributes"]["NORMAL"];
        uint32_t uvAccessor = primitive["attributes"]["TEXCOORD_0"];
        uint32_t indexAccessor = primitive["indices"];

        auto positions = GroupVec3(GetFloats(m_JSON["accessors"][posAccessor]));
        auto normals = GroupVec3(GetFloats(m_JSON["accessors"][normalAccessor]));
        auto texcoords = GroupVec2(GetFloats(m_JSON["accessors"][uvAccessor]));

        auto vertices = AssembleVertices(positions, normals, texcoords);

        auto indices = GetIndices(m_JSON["accessors"][indexAccessor]);

        auto textures = GetTextures();

        m_LoadedData.Meshes.push_back(
            Mesh::Create(vertices, indices)
        );
    }

    BufferBase GLTFModelLoader::GetBinaryData()
    {
        std::string uri = m_JSON["buffers"][0]["uri"];

        std::filesystem::path fullPath =
            m_Filepath.parent_path() / uri;

        return FileSystem::LoadFile(fullPath);
    }

    std::vector<float> GLTFModelLoader::GetFloats(const nlohmann::json& accessor)
    {
        std::vector<float> values;

        uint32_t bufferViewIndex = accessor["bufferView"];
        uint32_t count = accessor["count"];
        uint32_t byteOffset = accessor.value("byteOffset", 0);
        uint32_t componentType = accessor["componentType"];

        if (componentType != 5126)
            throw std::runtime_error("Unsupported component type");

        std::string type = accessor["type"];

        uint32_t componentCount = 0;

        if (type == "SCALAR") componentCount = 1;
        else if (type == "VEC2") componentCount = 2;
        else if (type == "VEC3") componentCount = 3;
        else if (type == "VEC4") componentCount = 4;
        else throw std::runtime_error("Unknown accessor type");

        auto bufferView = m_JSON["bufferViews"][bufferViewIndex];

        uint32_t bufferOffset = bufferView.value("byteOffset", 0);

        uint32_t stride =
            bufferView.value(
                "byteStride",
                componentCount * sizeof(float)
            );

        uint32_t start = bufferOffset + byteOffset;

        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t offset = start + i * stride;

            for (uint32_t j = 0; j < componentCount; j++)
            {
                float value;

                std::memcpy(
                    &value,
                    &m_Data.Data[offset + j * sizeof(float)],
                    sizeof(float)
                );

                values.push_back(value);
            }
        }

        return values;
    }

    std::vector<uint32_t> GLTFModelLoader::GetIndices(const nlohmann::json& accessor)
    {
        std::vector<uint32_t> indices;

        uint32_t bufferViewIndex = accessor["bufferView"];
        uint32_t count = accessor["count"];
        uint32_t byteOffset = accessor.value("byteOffset", 0);
        uint32_t componentType = accessor["componentType"];

        auto bufferView = m_JSON["bufferViews"][bufferViewIndex];

        uint32_t bufferOffset = bufferView.value("byteOffset", 0);

        uint32_t start = bufferOffset + byteOffset;

        switch (componentType)
        {
            case 5125:
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    uint32_t value;

                    std::memcpy(
                        &value,
                        &m_Data.Data[start + i * 4],
                        sizeof(uint32_t)
                    );

                    indices.push_back(value);
                }

                break;
            }

            case 5123:
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    uint16_t value;

                    std::memcpy(
                        &value,
                        &m_Data.Data[start + i * 2],
                        sizeof(uint16_t)
                    );

                    indices.push_back(value);
                }

                break;
            }

            default:
                throw std::runtime_error("Unsupported index format");
        }

        return indices;
    }

    std::vector<std::pair<TextureType, Ref<Texture2D>>>
    GLTFModelLoader::GetTextures()
    {
        std::vector<std::pair<TextureType, Ref<Texture2D>>> textures;

        if (!m_JSON.contains("images"))
            return textures;

        std::filesystem::path directory = m_Filepath.parent_path();

        for (auto& image : m_JSON["images"])
        {
            std::string uri = image["uri"];

            bool alreadyLoaded = false;

            for (uint32_t i = 0; i < m_LoadedTextureNames.size(); i++)
            {
                if (m_LoadedTextureNames[i] == uri)
                {
                    textures.push_back(m_LoadedTextures[i]);
                    alreadyLoaded = true;
                    break;
                }
            }

            if (alreadyLoaded)
                continue;

            TextureType type = TextureType::Diffuse;

            if (uri.find("metallicRoughness") != std::string::npos)
                type = TextureType::Specular;

            auto texture = Texture2D::Create((directory / uri).string().c_str());

            std::pair<TextureType, Ref<Texture2D>> pair =
            {
                type,
                texture
            };

            textures.push_back(pair);

            m_LoadedTextures.push_back(pair);
            m_LoadedTextureNames.push_back(uri);
        }

        return textures;
    }

    std::vector<MeshVertex> GLTFModelLoader::AssembleVertices(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& texcoords
    )
    {
        std::vector<MeshVertex> vertices;

        vertices.reserve(positions.size());

        for (size_t i = 0; i < positions.size(); i++)
        {
            MeshVertex vertex;

            vertex.position = positions[i];
            vertex.normal = normals[i];
            vertex.colour = glm::vec4(1.0f);
            vertex.uv = texcoords[i];

            vertices.push_back(vertex);
        }

        return vertices;
    }

    std::vector<glm::vec2>
    GLTFModelLoader::GroupVec2(const std::vector<float>& values)
    {
        std::vector<glm::vec2> result;

        for (size_t i = 0; i < values.size(); i += 2)
            result.emplace_back(values[i], values[i + 1]);

        return result;
    }

    std::vector<glm::vec3>
    GLTFModelLoader::GroupVec3(const std::vector<float>& values)
    {
        std::vector<glm::vec3> result;

        for (size_t i = 0; i < values.size(); i += 3)
            result.emplace_back(values[i], values[i + 1], values[i + 2]);

        return result;
    }

    std::vector<glm::vec4>
    GLTFModelLoader::GroupVec4(const std::vector<float>& values)
    {
        std::vector<glm::vec4> result;

        for (size_t i = 0; i < values.size(); i += 4)
            result.emplace_back(values[i], values[i + 1], values[i + 2], values[i + 3]);

        return result;
    }
}