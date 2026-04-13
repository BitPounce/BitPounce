#include <bp_pch.h>

#include "FontImporter.h"

namespace BitPounce
{
    Ref<Font> FontImporter::ImportFont(AssetHandle handle, const AssetMetadata &metadata)
    {
        return CreateRef<Font>(metadata.FilePath);
    }
}