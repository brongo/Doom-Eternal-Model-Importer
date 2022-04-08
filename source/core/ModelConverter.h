#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cmath>

#include "types/LWO.h"
#include "types/OBJ.h"
#include "types/ResourceFile.h"

#include "Oodle.h"
#include "ResourceFileReader.h"

#include "vendor/obj/obj.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    class ModelConverter
    {
        public:

            int VertexCount = 0;

            bool LoadResource(const std::string fileName);
            bool HasResourceLoadError() { return _HasResourceLoadError; }
            std::vector<ResourceEntry> GetResourceData() { return _ResourceData; }
            std::string GetLastErrorMessage() { return _LastErrorMessage; }
            std::string GetLastErrorDetail() { return _LastErrorDetail; }

            fs::path ExtractLWOHeader(fs::path lwoPath, fs::path resourcePath, bool temporaryExtraction, std::string streamDBIndexStr);
            std::vector<std::string> GetOBJMeshInfo(fs::path objPath);
            std::vector<std::string> GetLWOMeshInfo(fs::path lwoPath, fs::path resourcePath);
            int ConvertOBJtoLWO(fs::path gamePath, fs::path objPath, fs::path lwoPath, fs::path resourcePath, std::string material2decl, bool useYOrientation);

        private:

            bool _HasFatalError = 0;
            bool _HasResourceLoadError = 0;
            std::string _LastErrorMessage;
            std::string _LastErrorDetail;
            std::string _BasePath;
            std::string _ResourcePath;
            std::vector<ResourceEntry> _ResourceData;

            // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
            void ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail = "");
    };
}
