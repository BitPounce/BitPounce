#include "bp_pch.h"
#include "FileSystem.h"
#include "BitPounce/Core/Logger.h"

#include <fstream>
#include <unordered_map>

namespace BitPounce
{
	static std::unordered_map<std::filesystem::path, DiskBuffer> s_RealFiles;
	static std::unordered_map<std::filesystem::path, Buffer>     s_FakeRamFiles;
	static std::unordered_map<std::filesystem::path, DiskBuffer> s_FakeDiskFiles;
	static std::unordered_map<std::filesystem::path, Buffer> s_TEMP_BUFFERS;
	static std::unordered_map<std::filesystem::path, DiskBuffer> s_TEMP_DISK_BUFFERS;

	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		BP_CORE_INFO("Reading file into RAM: {}", filepath.string());

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		if (!stream) {
			BP_CORE_ERROR("Failed to open file: {}", filepath.string());
			return {};
		}

		std::streampos end = stream.tellg();
		if (end <= 0) {
			BP_CORE_WARN("File is empty or error occurred: {}", filepath.string());
			return {};
		}

		stream.seekg(0, std::ios::beg);
		uint64_t size = static_cast<uint64_t>(end);
		Buffer buffer(size);

		stream.read(buffer.As<char>(), size);
		if (!stream) {
			BP_CORE_ERROR("Failed to read file completely: {}", filepath.string());
			return {};
		}

		BP_CORE_INFO("Successfully read file into RAM: {}", filepath.string());
		return buffer;
	}

	DiskBuffer FileSystem::ReadFileBinaryDisk(const std::filesystem::path& filepath)
	{
		BP_CORE_INFO("Reading file from disk: {}", filepath.string());

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		if (!stream) {
			BP_CORE_ERROR("Failed to open file: {}", filepath.string());
			return {};
		}

		std::streampos end = stream.tellg();
		if (end <= 0) {
			BP_CORE_WARN("File is empty or error occurred: {}", filepath.string());
			return {};
		}

		stream.seekg(0, std::ios::beg);
		uint64_t size = static_cast<uint64_t>(end);
		DiskBuffer buffer(size);

		stream.read(buffer.As<char>(), size);
		if (!stream) {
			BP_CORE_ERROR("Failed to read file completely: {}", filepath.string());
			return {};
		}

		BP_CORE_INFO("Successfully read file from disk: {}", filepath.string());
		return buffer;
	}

	BufferBase FileSystem::LoadFile(const std::filesystem::path& filepath)
	{
		BP_CORE_INFO("Loading file: {}", filepath.string());

    	auto ramIt = s_FakeRamFiles.find(filepath);
    	if (ramIt != s_FakeRamFiles.end()) {
			BP_CORE_INFO("Found fake RAM file: {}", filepath.string());
        	return ramIt->second;
    	}

    	auto diskIt = s_FakeDiskFiles.find(filepath);
    	if (diskIt != s_FakeDiskFiles.end()) {
			BP_CORE_INFO("Found fake Disk file: {}", filepath.string());
    	    return diskIt->second;
    	}

    	auto realIt = s_RealFiles.find(filepath);
    	if (realIt != s_RealFiles.end()) {
			BP_CORE_INFO("Found real file in cache: {}", filepath.string());
    	    return realIt->second;
    	}

    	DiskBuffer fileBuffer = ReadFileBinaryDisk(filepath);
    	if (!fileBuffer) {
			BP_CORE_ERROR("Failed to load file: {}", filepath.string());
    	    return {};
    	}

		BP_CORE_INFO("Loaded file successfully: {}", filepath.string());
    	return fileBuffer;
	}

    BufferBase FileSystem::AddFakeFile(const std::filesystem::path &filepath, const Buffer &buffer)
    {
		BP_CORE_INFO("Adding fake RAM file: {}", filepath.string());
		s_FakeRamFiles[filepath] = Buffer::Copy(buffer);
		Buffer stored = s_FakeRamFiles[filepath];
		return stored;
	}

	BufferBase FileSystem::AddFakeFile(const std::filesystem::path& filepath, const DiskBuffer& buffer)
	{
		BP_CORE_INFO("Adding fake Disk file: {}", filepath.string());
		s_FakeDiskFiles[filepath] = DiskBuffer::Copy(buffer);
		DiskBuffer stored = s_FakeDiskFiles[filepath];
		return stored;
	}

	DiskBuffer FileSystem::AddFile(const std::filesystem::path& filepath)
	{
		BP_CORE_INFO("Adding real file: {}", filepath.string());
		DiskBuffer buffer = ReadFileBinaryDisk(filepath);
		if (!buffer) {
			BP_CORE_ERROR("Failed to read real file: {}", filepath.string());
			return {};
		}

		s_RealFiles[filepath] = buffer;
		BP_CORE_INFO("Successfully added real file: {}", filepath.string());
		return buffer;
	}
}
