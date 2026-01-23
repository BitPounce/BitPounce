#pragma once

#include "BitPounce/Core/Buffer.h"
#include <filesystem>


namespace BitPounce {

	class FileSystem
	{
	public:
		// TODO: move to FileSystem class
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
        static DiskBuffer ReadFileBinaryDisk(const std::filesystem::path& filepath);

		static BufferBase LoadFile(const std::filesystem::path& filepath);
		
		static BufferBase AddFakeFile(const std::filesystem::path& filepath, const Buffer& buffer);
		static BufferBase AddFakeFile(const std::filesystem::path& filepath, const DiskBuffer& buffer);
		static DiskBuffer AddFile(const std::filesystem::path& filepath);
	};

}