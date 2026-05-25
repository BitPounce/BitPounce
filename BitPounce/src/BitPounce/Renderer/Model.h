#pragma once

#include <filesystem>

#include "Mesh.h"
#include "IModelLoader.h"

namespace BitPounce
{
    class Model
    {
    public:
        Model(const std::filesystem::path& filepath);

        static Ref<Model> Create(const std::filesystem::path& filepath);

        const std::vector<Ref<Mesh>>& GetMeshes() const
        {
            return m_Meshes;
        }

        const std::vector<glm::mat4>& GetMatrices() const
        {
            return m_Matrices;
        }

    private:
        std::filesystem::path m_Filepath;

        std::vector<Ref<Mesh>> m_Meshes;

        std::vector<glm::vec3> m_Translations;
        std::vector<glm::quat> m_Rotations;
        std::vector<glm::vec3> m_Scales;
        std::vector<glm::mat4> m_Matrices;

		friend class RenderCommand;
    };
}