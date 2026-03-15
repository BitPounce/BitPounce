#include <bp_pch.h>
#include "BitPounce/Core/PlatformTools.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>
#include "BitPounce/Utils/PlatformUtils.h"

#include <random>
#include <sstream>

namespace uuid {
	static std::random_device              rd;
	static std::mt19937                    gen(rd());
	static std::uniform_int_distribution<> dis(0, 15);
	static std::uniform_int_distribution<> dis2(8, 11);

	std::string generate_uuid_v4() {
		std::stringstream ss;
		int i;
		ss << std::hex;
		for (i = 0; i < 8; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 4; i++) {
			ss << dis(gen);
		}
		ss << "-4";
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		ss << dis2(gen);
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 12; i++) {
			ss << dis(gen);
		};
		return ss.str();
	}
}

struct PageFile
{
	size_t size;
	void* address;
	std::string filename;
	int fd;
	
	// Constructor for initialization
	PageFile() : size(0), address(nullptr), fd(-1) {}
	
	// Destructor to ensure cleanup
	~PageFile() {
		if (fd != -1) {
			close(fd);
			fd = -1;
		}
	}
};

// Global registry - storing PageFile objects directly
static std::vector<PageFile> g_pageFiles;

// Cleanup function to delete all page files on exit
static void cleanupPageFiles() {
	for (auto& pf : g_pageFiles) {
		// Sync to disk
		if (pf.address) {
			msync(pf.address, pf.size, MS_SYNC);
			munmap(pf.address, pf.size);
			pf.address = nullptr;
		}
		
		// Delete the file
		if (!pf.filename.empty()) {
			unlink(pf.filename.c_str());
			pf.filename.clear();
		}
		
		// File descriptor closed in destructor when pf goes out of scope
	}
	g_pageFiles.clear();
}

// Register cleanup at program start
static struct PageFileCleanup {
	PageFileCleanup() {
		atexit(cleanupPageFiles);
	}
} pageFileCleanup;

void* DiskAlloc(size_t size, void* address)
{
	// Validate size
	if (size == 0) {
		return nullptr;
	}
	
	// Create temporary file with UUID
	std::string uuid_str = uuid::generate_uuid_v4();
	std::string filename = "/tmp/bp_page_" + uuid_str;
	
	// Open file with O_EXCL to ensure it doesn't exist
	int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
	if (fd == -1) {
		return nullptr;
	}
	
	// Allocate disk space
	if (ftruncate(fd, size) == -1) {
		close(fd);
		unlink(filename.c_str());
		return nullptr;
	}
	
	// Map file to memory
	void* mapped_addr = mmap(
		address,
		size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd,
		0
	);
	
	if (mapped_addr == MAP_FAILED) {
		close(fd);
		unlink(filename.c_str());
		return nullptr;
	}
	
	// Create PageFile object and add to registry
	PageFile pf;
	pf.size = size;
	pf.address = mapped_addr;
	pf.filename = filename;  // Keep filename for later deletion
	pf.fd = fd;
	
	// Add to vector
	g_pageFiles.push_back(pf);
	
	return mapped_addr;
}

void DiskFree(size_t size, void* address)
{
	if (!address) return;
	
	// Find the PageFile in the registry
	for (auto it = g_pageFiles.begin(); it != g_pageFiles.end(); ++it) {
		if (it->address == address) {
			// Sync to disk
			msync(it->address, it->size, MS_SYNC);
			
			// Unmap memory
			munmap(it->address, it->size);
			it->address = nullptr;
			
			// Delete the file (do NOT delete immediately after mmap)
			if (!it->filename.empty()) {
				unlink(it->filename.c_str());
				it->filename.clear();
			}
			
			// Close file descriptor (will happen in destructor when erased)
			if (it->fd != -1) {
				close(it->fd);
				it->fd = -1;
			}
			
			// Remove from registry
			g_pageFiles.erase(it);
			return;
		}
	}
	
	// Address not found in our registry
	// Just unmap it (might be memory allocated elsewhere)
	munmap(address, size);
}

namespace BitPounce
{
	// Converts Windows-style filter "*.txt;*.cpp" into Zenity --file-filter string
	static std::string PatternToZenityFilter(const std::string& pattern)
	{
		std::stringstream ss(pattern);
		std::string item;
		std::vector<std::string> filters;

		while (std::getline(ss, item, ';'))
		{
			if (!item.empty())
			{
				filters.push_back("\"" + item + "\"");
			}
		}

		// Join with spaces (Zenity expects space-separated globs)
		std::string result;
		for (size_t i = 0; i < filters.size(); ++i)
		{
			if (i > 0) result += " ";
			result += filters[i];
		}

		return result;
	}

	static std::string WindowsFilterToZenity(const char* winFilter)
	{
		std::string result;
		const char* p = winFilter;
		bool firstEntry = true;

		while (*p)
		{
			std::string description = p;
			p += description.size() + 1;

			std::string pattern = p;
			p += pattern.size() + 1;

			std::string extensions = PatternToZenityFilter(pattern);

			if (!firstEntry) result += " ";

			// Zenity syntax: "TEXT | *.ext1 *.ext2"
			result += "\"" + description + " | " + extensions + "\"";

			firstEntry = false;
		}

		return result;
	}

	// Helper to execute a command and capture stdout
	static std::string ExecCommand(const std::string& cmd)
	{
		std::string result;
		std::array<char, 128> buffer;

		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe) return "";

		while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
		{
			result += buffer.data();
		}

		pclose(pipe);

		// Remove trailing newline
		if (!result.empty() && result.back() == '\n')
			result.pop_back();

		return result;
	}

	std::string FileDialogs::OpenFile(const char* winFilter)
	{
		std::string filter = WindowsFilterToZenity(winFilter);

		// Zenity command
		std::string cmd = "zenity --file-selection --title=\"Open File\" --file-filter=" + filter;
		return ExecCommand(cmd);
	}

	std::string FileDialogs::SaveFile(const char* winFilter)
	{
		std::string filter = WindowsFilterToZenity(winFilter);

		// Zenity command
		std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=\"Save File\" --file-filter=" + filter;
		return ExecCommand(cmd);
	}
}