#include <bp_pch.h>
#include "BitPounce/Core/PlatformTools.h"

#include <Windows.h>
#include <vector>
#include <string>
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
    
    // Constructor for initialization
    PageFile() : size(0), address(nullptr) {}
};

// Global registry - storing PageFile objects directly
static std::vector<PageFile> g_pageFiles;

// Cleanup function to free all allocated memory on exit
static void cleanupPageFiles() {
    for (auto& pf : g_pageFiles) {
        if (pf.address) {
            // Free the virtual memory
            VirtualFree(pf.address, 0, MEM_RELEASE);
            pf.address = nullptr;
        }
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
    
    // Allocate memory that will be backed by the system page file
    // MEM_COMMIT: Allocates physical storage in memory or in the paging file on disk
    // PAGE_READWRITE: Memory can be read from and written to
    void* mapped_addr = VirtualAlloc(
        address,            // Desired base address (can be NULL)
        size,               // Size in bytes
        MEM_RESERVE | MEM_COMMIT, // Reserve address space and commit physical storage
        PAGE_READWRITE      // Read/write access
    );
    
    if (mapped_addr == NULL) {
        return nullptr;
    }
    
    // Create PageFile object and add to registry
    PageFile pf;
    pf.size = size;
    pf.address = mapped_addr;
    pf.filename = "";  // VirtualAlloc doesn't use a file, it uses system page file
    
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
            // Free the virtual memory
            // MEM_RELEASE: Releases the entire region that was reserved by VirtualAlloc
            VirtualFree(it->address, 0, MEM_RELEASE);
            it->address = nullptr;
            
            // Remove from registry
            g_pageFiles.erase(it);
            return;
        }
    }
    
    // Address not found in our registry
    // Just free it anyway
    VirtualFree(address, 0, MEM_RELEASE);
}