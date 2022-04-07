#include "ResourceFileReader.h"

namespace HAYDEN
{
    // Retrieve embedded file data for a specific file entry
    std::vector<uint8_t> ResourceFileReader::GetEmbeddedFileHeader(FILE* f, const uint64_t fileOffset, const uint64_t compressedSize, const uint64_t decompressedSize)
    {
        std::vector<uint8_t> embeddedHeader(compressedSize);
        fseek(f, (long)fileOffset, SEEK_SET);
        fread(embeddedHeader.data(), 1, compressedSize, f);

        if (embeddedHeader.size() != decompressedSize)
            embeddedHeader = oodleDecompress(embeddedHeader, decompressedSize);

        return embeddedHeader;
    }

    // Retrieve all entries from a .resources file as vector<ResourceEntry>
    std::vector<ResourceEntry> ResourceFileReader::ParseResourceFile()
    {
        // read .resources file from filesystem
        ResourceFile resourceFile(ResourceFilePath);
        std::vector<uint64_t> pathStringIndexes = resourceFile.GetAllPathStringIndexes();
        uint32_t numFileEntries = resourceFile.GetNumFileEntries();

        // allocate vector to hold all entries from this .resources file
        std::vector<ResourceEntry> resourceData;
        resourceData.resize(numFileEntries);

        // Parse each resource file and convert to usable data
        for (uint32_t i = 0; i < numFileEntries; i++)
        {
            ResourceFileEntry& lexedEntry = resourceFile.GetResourceFileEntry(i);
            resourceData[i].DataOffset = lexedEntry.DataOffset;
            resourceData[i].DataSize = lexedEntry.DataSize;
            resourceData[i].DataSizeUncompressed = lexedEntry.DataSizeUncompressed;
            resourceData[i].Version = lexedEntry.Version;
            resourceData[i].StreamResourceHash = lexedEntry.StreamResourceHash;
            resourceData[i].Type = resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index]);
            resourceData[i].Name = resourceFile.GetResourceStringEntry(pathStringIndexes[lexedEntry.PathTuple_Index + 1]);
        }

        return resourceData;
    };

    // Finds the ResourceID for a specific file
    uint64_t ResourceFileReader::GetResourceIndex(fs::path targetResourceEntry)
    {
        // get resource file entries
        std::vector<ResourceEntry> resourceEntries = ParseResourceFile();

        // find the resource entry we're looking for
        ResourceEntry targetEntry;
        for (int i = 0; i < resourceEntries.size(); i++)
        {
            if (resourceEntries[i].Name != targetResourceEntry)
                continue;

            // found, stop looking
            targetEntry = resourceEntries[i];
            break;
        }

        return targetEntry.StreamResourceHash;
    }

    // Convert resource entry ID to streamdb entry ID
    uint64_t ResourceFileReader::CalculateStreamDBIndex(uint64_t resourceId, int mipCount)
    {
        // Get hex bytes string
        std::string hexBytes = intToHex(resourceId);

        // Reverse each byte
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t)1]);

        // Shift digits to the right
        hexBytes = hexBytes.substr(hexBytes.size() - 1) + hexBytes.substr(0, hexBytes.size() - 1);

        // Reverse each byte again
        for (int i = 0; i < hexBytes.size(); i += 2)
            std::swap(hexBytes[i], hexBytes[i + (int64_t)1]);

        // Get second digit based on mip count
        hexBytes[1] = intToHex((char)(6 + mipCount))[1];

        // Convert hex string back to uint64 and return
        return hexToInt64(hexBytes);
    }

}