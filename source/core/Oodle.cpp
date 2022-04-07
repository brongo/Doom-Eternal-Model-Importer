#include "Oodle.h"

namespace HAYDEN
{
    // Decompress using Oodle DLL
    OodLZ_DecompressFunc* OodLZ_Decompress = NULL;
    OodLZ_CompressFunc* OodLZ_Compress = NULL;

    bool oodleInit(const std::string& basePath)
    {
        std::string oodlePath = basePath.substr(0, basePath.length() - 4) + "oo2core_8_win64.dll";
        bool test = 0;
#ifdef _WIN32
        // Load oodle dll
        auto oodle = LoadLibraryA(oodlePath.c_str());
        if (!oodle)
            return false;

        OodLZ_Decompress = (OodLZ_DecompressFunc*)GetProcAddress(oodle, "OodleLZ_Decompress");
        OodLZ_Compress = (OodLZ_CompressFunc*)GetProcAddress(oodle, "OodleLZ_Compress");
#else
        // Copy oodle to current dir to prevent linoodle errors
        std::error_code ec;
        fs::copy(oodlePath, fs::current_path(), ec);
        if (ec.value() != 0)
            return false;

        // Load linoodle library
        std::string linoodlePath = basePath + "/liblinoodle.so";
        auto oodle = dlopen(linoodlePath.c_str(), RTLD_LAZY);
        OodLZ_Decompress = (OodLZ_DecompressFunc*)dlsym(oodle, "OodleLZ_Decompress");
        OodLZ_Compress = (OodLZ_CompressFunc*)dlsym(oodle, "OodleLZ_Compress");

        // Remove oodle dll
        fs::remove(fs::current_path().append("oo2core_8_win64.dll"), ec);
#endif

        if (oodle == NULL || OodLZ_Decompress == NULL)
            return false;

        return true;
    }

    std::vector<uint8_t> oodleDecompress(std::vector<uint8_t> compressedData, const uint64_t decompressedSize)
    {
        if (OodLZ_Decompress == NULL)
            return std::vector<uint8_t>();

        std::vector<uint8_t> output(decompressedSize + SAFE_SPACE);
        uint64_t outbytes = 0;

        // Decompress using Oodle DLL
        outbytes = OodLZ_Decompress(compressedData.data(), compressedData.size(), output.data(), decompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        if (outbytes == 0)
        {
            fprintf(stderr, "Error: failed to decompress with Oodle DLL.\n\n");
            return std::vector<uint8_t>();
        }

        return std::vector<uint8_t>(output.begin(), output.begin() + outbytes);
    }

    int oodleCompress(std::string filename, std::string destFilename) 
    {
        FILE* f;
        uint8_t* input = NULL;
        uint8_t* output = NULL;
        uint64_t unpacked_size = 0;
        uint64_t compressed_size = 0;

        // Make sure we have Oodle DLL available
        if (OodLZ_Compress == NULL)
        {
            fprintf(stderr, "ERROR : OodleLibrary : Oodle has not been initialized. Run Oodle::Init() first. \n\n");
            return false;
        }

        // Open source file for reading
        f = fopen(filename.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : OodleLibrary : Failed to open %s for reading! \n\n", filename.c_str());
            return 1;
        }

        // Compress file in memory
        if (f != NULL)
        {
            fseek(f, 0, SEEK_END);

            unpacked_size = ftell(f);     
            input = new uint8_t[unpacked_size];
            output = new uint8_t[unpacked_size + 65536];

            fseek(f, 0, SEEK_SET);

            if (fread(input, 1, unpacked_size, f) != unpacked_size) 
            {
                fprintf(stderr, "ERROR : OodleLibrary : Failed to read file into memory.\n");
                return 1;
            }

            if (!output)
            {
                fprintf(stderr, "ERROR : OodleLibrary : Couldn't allocate memory for compression.\n");
                return 1;
            }

            // 8 = Kraken, 4 = compression level
            compressed_size = OodLZ_Compress(8, input, unpacked_size, output, 4, 0, 0, 0, 0, 0);

            fclose(f);
        }

        // Make sure compression was successful
        if (compressed_size <= 0) 
        {
            fprintf(stderr, "ERROR : OodleLibrary : Compression failed.\n");
            return 1;
        }

        // Write compressed data to destination file
        f = fopen(destFilename.c_str(), "wb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : OodleLibrary : Failed to open destination file for writing!\n");
            return 1;
        }

        if (f != NULL && output != NULL) 
        {
            // WRITE OUR STREAMDB MAGIC HEADER - static for now
            uint64_t streamDBMagic = 4775026447650804819;
            uint32_t lodCount = 3;
            uint32_t lodDataOffset = 36;
            uint32_t lodDataLength = compressed_size;

            fwrite(&streamDBMagic, 8, 1, f);
            fwrite(&lodCount, 4, 1, f);

            for (int i = 0; i < lodCount; i++)
            {
                fwrite(&lodDataOffset, 4, 1, f);
                fwrite(&lodDataLength, 4, 1, f);
            }

            fwrite(output, 1, compressed_size, f);
            fclose(f);
        }

        return compressed_size;
    }
}