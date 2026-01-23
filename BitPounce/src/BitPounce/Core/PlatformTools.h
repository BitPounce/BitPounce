#pragma once
#include "BitPounce/Core/Base.h"

void* DiskAlloc(size_t size, void* address = NULL);
void DiskFree(size_t size, void* address);
void* DiskMemCpy(void* dest, const void* src, size_t size);
