#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <string>
#include <vector>
#include <filesystem>

#define SAFE_SPACE 64

typedef int OodLZ_DecompressFunc(
    uint8_t* src_buf, int src_len, uint8_t* dst, size_t dst_size, int fuzz, int crc, int verbose,
    uint8_t* dst_base, size_t e, void* cb, void* cb_ctx, void* scratch, size_t scratch_size, int threadPhase);

typedef int OodLZ_CompressFunc(
    int codec, uint8_t* src_buf, size_t src_len, uint8_t* dst_buf, int level,
    void* opts, size_t offs, size_t unused, void* scratch, size_t scratch_size);

namespace fs = std::filesystem;

namespace HAYDEN
{
    // Decompress using Oodle DLL
    bool oodleInit(const std::string& basePath);
    std::vector<uint8_t> oodleDecompress(std::vector<uint8_t> compressedData, const uint64_t decompressedSize);
    int oodleCompress(std::string filename, std::string destFilename);
}
