#include <bp_pch.h>
#include "BitPounce/Core/PlatformTools.h"

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