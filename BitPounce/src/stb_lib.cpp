#include "bp_pch.h"

#include "BitPounce/Core/Buffer.h"
#include "BitPounce/Core/FileSystem.h"
#include <filesystem>


#define STB_INCLUDE_LOAD_FILE(text, filename, plen) \
    BitPounce::BufferBase buffer = BitPounce::FileSystem::LoadFile(std::filesystem::path(filename)); \
    text = (char*)malloc(buffer.Size + 1); \
    DiskMemCpy(text, buffer.As<void>(), buffer.Size); \
    text[buffer.Size] = '\0';

#define STB_INCLUDE_IMPLEMENTATION
#include "stb_include.h"

