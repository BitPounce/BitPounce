#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include <BitPounce/Audio/Audio.h>

namespace BitPounce
{
    class AudioImporter
    {
    public:
        static Ref<Audio> ImportAudio(AssetHandle handle, const AssetMetadata& metadata);
    };
}