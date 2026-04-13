#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include <BitPounce/Renderer/Font.h>

namespace BitPounce
{
    class FontImporter
    {
    public:
        static Ref<Font> ImportFont(AssetHandle handle, const AssetMetadata& metadata);
    };
}