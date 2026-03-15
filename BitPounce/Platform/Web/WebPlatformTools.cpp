#include <bp_pch.h>
#include "BitPounce/Core/PlatformTools.h"
#include "BitPounce/Utils/PlatformUtils.h"

void* DiskAlloc(size_t size, void* address)
{
    
    if (address)
    {
        BP_CORE_WARN("Do not use DiskAlloc on web builds, using placement new");
        return ::operator new(size, address); // placement new
    }
    else
    {
        BP_CORE_WARN("Do not use DiskAlloc on web builds, using regular allocation");
        return malloc(size); // regular allocation
    }
        
}
void DiskFree(size_t size, void* address)
{
    BP_CORE_WARN("Do not use DiskFree on web builds, using normal free");
    free(address);
}

namespace BitPounce
{
    std::string FileDialogs::OpenFile(const char* filter)
	{
		BP_CORE_ASSERT(0,"OpenFile is not supported on web");
		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		BP_CORE_ASSERT(0,"SaveFile is not supported on web");
		return std::string();
    }
}