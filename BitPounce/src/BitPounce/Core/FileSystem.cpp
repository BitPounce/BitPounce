#include "bp_pch.h"
#include "FileSystem.h"
#include "BitPounce/Core/Logger.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
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

		FILE* file = fopen(filepath.string().c_str(), "rb");
		if (!file) {
			BP_CORE_ERROR("Failed to open file: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			return {};
		}

		// Get file size
		if (fseek(file, 0, SEEK_END) != 0) {
			BP_CORE_ERROR("Failed to seek file: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			fclose(file);
			return {};
		}

		long size = ftell(file);
		if (size < 0) {
			BP_CORE_ERROR("Failed to get file position: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			fclose(file);
			return {};
		}

		if (size == 0) {
			BP_CORE_WARN("File is empty: {}", filepath.string());
			fclose(file);
			return {};
		}

		// Seek back to beginning
		rewind(file);

		Buffer buffer(static_cast<uint64_t>(size));
		size_t bytesRead = fread(buffer.As<char>(), 1, static_cast<size_t>(size), file);

		if (bytesRead != static_cast<size_t>(size)) {
			if (feof(file)) {
				BP_CORE_ERROR("Unexpected end of file while reading: {}. Read {} of {} bytes", 
							  filepath.string(), bytesRead, size);
			} else if (ferror(file)) {
				BP_CORE_ERROR("Error reading file: {} - Error: {}", 
							  filepath.string(), strerror(errno));
			}
			fclose(file);
			return {};
		}

		fclose(file);
		BP_CORE_INFO("Successfully read file into RAM: {} ({} bytes)", filepath.string(), size);
		return buffer;
	}

	DiskBuffer FileSystem::ReadFileBinaryDisk(const std::filesystem::path& filepath)
	{
		BP_CORE_INFO("Reading file from disk: {}", filepath.string());

		FILE* file = fopen(filepath.string().c_str(), "rb");
		if (!file) {
			BP_CORE_ERROR("Failed to open file: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			return {};
		}

		// Get file size
		if (fseek(file, 0, SEEK_END) != 0) {
			BP_CORE_ERROR("Failed to seek file: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			fclose(file);
			return {};
		}

		long size = ftell(file);
		if (size < 0) {
			BP_CORE_ERROR("Failed to get file position: {} - Error: {}", 
						  filepath.string(), strerror(errno));
			fclose(file);
			return {};
		}

		if (size == 0) {
			BP_CORE_WARN("File is empty: {}", filepath.string());
			fclose(file);
			return {};
		}

		// Seek back to beginning
		rewind(file);

		Buffer buffer(static_cast<uint64_t>(size));
		size_t bytesRead = fread(buffer.As<char>(), 1, static_cast<size_t>(size), file);

		if (bytesRead != static_cast<size_t>(size)) {
			if (feof(file)) {
				BP_CORE_ERROR("Unexpected end of file while reading: {}. Read {} of {} bytes", 
							  filepath.string(), bytesRead, size);
			} else if (ferror(file)) {
				BP_CORE_ERROR("Error reading file: {} - Error: {}", 
							  filepath.string(), strerror(errno));
			}
			fclose(file);
			return {};
		}

		fclose(file);
		BP_CORE_INFO("Successfully read file from disk: {} ({} bytes)", filepath.string(), size);
		// If it works its works.
		return buffer.ToBuffer();
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

	BufferBase FileSystem::AddFakeFile(const std::filesystem::path& filepath, const Buffer& buffer)
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