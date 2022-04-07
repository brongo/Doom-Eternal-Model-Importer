#pragma once

#include <string>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace HAYDEN
{
    // hex <-> decimal conversions and endian functions
    template <typename T>
    std::string intToHex(const T num)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +num;
        return stream.str();
    }
    uint64_t hexToInt64(const std::string hex);
    void endianSwap(uint64_t& value);

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows
    bool mkpath(const fs::path& path);

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE* openLongFilePath(const fs::path& path);
}