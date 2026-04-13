#include <bp_pch.h>

#include "AudioImporter.h"

namespace BitPounce
{
    Ref<Audio> AudioImporter::ImportAudio(AssetHandle handle, const AssetMetadata &metadata)
    {
        // just call Audio::Create
        return Audio::Create(metadata.FilePath);
    }
}