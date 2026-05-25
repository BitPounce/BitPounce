#include <bp_pch.h>
#include "Model.h"

#include "GLTFModelLoader.h"

namespace BitPounce
{
    Model::Model(const std::filesystem::path& filepath)
        : m_Filepath(filepath)
    {
        GLTFModelLoader loader;

        if (!loader.CanLoad(filepath))
            throw std::runtime_error("Unsupported model format");

        ModelLoadedData data = loader.Load(filepath);

        m_Meshes = std::move(data.Meshes);
        m_Translations = std::move(data.Translations);
        m_Rotations = std::move(data.Rotations);
        m_Scales = std::move(data.Scales);
        m_Matrices = std::move(data.Matrices);
    }

    Ref<Model> Model::Create(const std::filesystem::path& filepath)
    {
        return CreateRef<Model>(filepath);
    }
}