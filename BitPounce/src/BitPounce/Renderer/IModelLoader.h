#pragma once

#include <filesystem>
#include <vector>

#include "Mesh.h"

namespace BitPounce
{
    struct ModelLoadedData
    {
        std::vector<Ref<Mesh>> Meshes;

        std::vector<glm::vec3> Translations;
        std::vector<glm::quat> Rotations;
        std::vector<glm::vec3> Scales;
        std::vector<glm::mat4> Matrices;
    };

    class IModelLoader
    {
    public:
        virtual ~IModelLoader() = default;

        virtual bool CanLoad(const std::filesystem::path& filepath) const = 0;

        virtual ModelLoadedData Load(const std::filesystem::path& filepath) = 0;
    };
}