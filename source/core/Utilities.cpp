#include "Utilities.h"

namespace HAYDEN
{
    void endianSwap(uint64_t& value)
    {
        value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
        value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
        value = ((value & 0x00FF00FF00FF00FFull) << 8) | ((value & 0xFF00FF00FF00FF00ull) >> 8);
    }

    uint64_t hexToInt64(const std::string hex)
    {
        uint64_t x;
        std::stringstream stream;
        stream << std::hex << hex;
        stream >> x;
        return x;
    }

    // Recursive mkdir, bypassing PATH_MAX limitations on Windows       
    bool mkpath(const fs::path& path)
    {
        std::error_code ec;

#ifdef _WIN32
        // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
        // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details 
        fs::create_directories(L"\\\\?\\" + fs::absolute(path).wstring(), ec);
#else
        fs::create_directories(path, ec);
#endif
        return ec.value() == 0;
    }

    // Opens FILE* with long filepath, bypasses PATH_MAX limitations in Windows
    FILE* openLongFilePath(const fs::path& path)
    {
#ifdef _WIN32
        // "\\?\" alongside the wide string functions is used to bypass PATH_MAX
        // Check https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for details 
        std::wstring wPath = L"\\\\?\\" + path.wstring();
        FILE* file = _wfopen(wPath.c_str(), L"wb");
#else
        FILE* file = fopen(path.c_str(), "wb");
#endif
        return file;
    }
}