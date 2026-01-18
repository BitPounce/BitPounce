#pragma once
#include "BitPounce/Core/Base.h"

void* DiskAlloc(size_t size, void* address = NULL);
void DiskFree(size_t size, void* address);