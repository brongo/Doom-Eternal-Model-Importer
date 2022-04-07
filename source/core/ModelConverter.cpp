#include "ModelConverter.h"

using namespace HAYDEN;

namespace HAYDEN
{
    void ModelConverter::ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail)
    {
        _LastErrorMessage = errorMessage;
        _LastErrorDetail = errorDetail;

        if (isFatal)
            _HasFatalError = 1;

        std::string consoleMsg = _LastErrorMessage + " " + _LastErrorDetail + "\n";
        fprintf(stderr, "%s", consoleMsg.c_str());
        return;
    }

    bool ModelConverter::LoadResource(const std::string resourcePath)
    {
        // Get base path from resource path
        auto baseIndex = resourcePath.find("base");
        if (baseIndex == -1)
        {
            ThrowError(1,
                "Failed to load .resource file.",
                "The .resource file must be located in your Doom Eternal \"base\" directory or its subdirectories."
            );
            _HasResourceLoadError = 1;
            return 0;
        }
        _BasePath = resourcePath.substr(0, baseIndex + 4);

        // Make sure we have Oodle DLL available
        if (!oodleInit(_BasePath))
        {
            ThrowError(1,
                "Failed to load the oodle dll.",
                "Make sure the oo2core_8_win64.dll file is present in your game directory."
            );
            _HasResourceLoadError = 1;
            return 0;
        }

        // Load the currently requested .resources file
        _ResourcePath = resourcePath;
        _HasResourceLoadError = 0;
        try
        {
            if (resourcePath.rfind(".resources") == -1)
            {
                ThrowError(0, "Not a valid .resources file.", "Please load a file with the .resources or .resources.backup file extension.");
                _HasResourceLoadError = 1;
                return 0;
            }

            // load .resources file data
            ResourceFileReader reader(_ResourcePath);
            _ResourceData = reader.ParseResourceFile();
        }
        catch (...)
        {
            ThrowError(0, "Failed to read .resources file.", "Please load a file with the .resources or .resources.backup file extension.");
            _HasResourceLoadError = 1;
            return 0;
        }

        return 1;
    }

    fs::path ModelConverter::ExtractLWOHeader(fs::path lwoPath, fs::path resourcePath, bool temporaryExtraction, std::string streamDBIndexStr = "")
    {
        // parse resource file
        ResourceFileReader resourceReader(resourcePath);
        std::vector<ResourceEntry> resourceEntries = resourceReader.ParseResourceFile();

        // find the modelFullName we're looking for
        ResourceEntry targetEntry;
        for (int i = 0; i < resourceEntries.size(); i++)
        {
            if (resourceEntries[i].Name != lwoPath)
                continue;

            // found, stop looking
            targetEntry = resourceEntries[i];
            break;
        }

        // extract the header
        std::vector<uint8_t> targetData;
        FILE* f = fopen(resourcePath.string().c_str(), "rb");

        if (f != NULL)
        {
            targetData = resourceReader.GetEmbeddedFileHeader(f, targetEntry.DataOffset, targetEntry.DataSize, targetEntry.DataSizeUncompressed);
            fclose(f);
        }

        fs::path lwoFile = lwoPath.filename();
        fs::path modelHeader = lwoFile;

        // create resource directory for file export
        if (!temporaryExtraction)
        {
            // create import and streamdb folders
            fs::path importPath = "imports";
            std::string lwoFileNameForImportPath = lwoPath.filename().replace_extension("").string();
            fs::path thisImportPath = importPath / fs::path(lwoFileNameForImportPath + "_id#" + streamDBIndexStr);
            fs::path resourceName = resourcePath.filename().replace_extension("").replace_extension("");    // twice in case of resources.backup
            fs::path resourceImportPath = thisImportPath / resourceName;

            if (!fs::exists(resourceImportPath))
                if (!mkpath(resourceImportPath))
                    fprintf(stderr, "Error: Failed to create directories for file: %s \n", resourceImportPath.string().c_str());


            // create model path within resource directory
            fs::path modelPath = resourceImportPath / lwoPath.remove_filename();
            modelPath.make_preferred();

            if (!fs::exists(modelPath))
                if (!mkpath(modelPath))
                    fprintf(stderr, "Error: Failed to create directories for file: %s \n", modelPath.string().c_str());

            modelHeader = modelPath / lwoFile;
        }
        else
        {
            modelHeader = lwoFile;
        }

        // write to filesystem
        std::string modelHeaderStr = modelHeader.string();
        fs::path modelHeaderPathWideStr = fs::current_path() / fs::path(modelHeaderStr); // OK this fixes it not finding the file, I guess it needs a full path

        FILE* wf = openLongFilePath(modelHeaderPathWideStr); //wb
        if (wf != NULL)
        {
            fwrite(targetData.data(), 1, targetData.size(), wf);
            fclose(wf);
        }

        return modelHeaderStr;
    }

    std::vector<std::string> ModelConverter::GetOBJMeshInfo(fs::path objPath)
    {
        OBJFile objFile(objPath);
        std::vector<std::string> meshInfo;

        // Read mesh data into vector
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            meshInfo.push_back(objFile.Objects[i].ObjectName);
        }

        // Return early if too many meshes
        if (meshInfo.size() > 1)
        {
            return meshInfo;
        }

        // Make sure vertex data is readable
        for (int i = 0; i < objFile.Objects[0].Vertices.size(); i++)
        {
            if (objFile.Objects[0].Vertices[i].rfind("e+") != -1)
            {
                meshInfo.resize(0);
                return meshInfo;
            }
        }

        return meshInfo;
    }

    std::vector<std::string> ModelConverter::GetLWOMeshInfo(fs::path lwoPath, fs::path resourcePath)
    {
        LWO LWOHeader;
        std::vector<std::string> meshInfo;

        // Extract and serialize .lwo header file
        fs::path localLWOPath = ExtractLWOHeader(lwoPath, resourcePath, 1);
        LWOHeader.Serialize(localLWOPath);

        // Remove temporary extracted file
        fs::remove(localLWOPath);

        // This checks for a parse error - if our decl strlen is too long or < 0 we know it messed up somewhere. Abort.
        int64_t lastIndex = LWOHeader.MeshData.size();
        if (LWOHeader.MeshData[lastIndex - 1].MeshHeader.DeclStrlen < 0 || LWOHeader.MeshData[lastIndex - 1].MeshHeader.DeclStrlen > 1024)
        {
            return meshInfo;
        }

        // Read mesh data into vector
        for (int i = 0; i < LWOHeader.Header.NumMeshes; i++)
        {
            meshInfo.push_back(LWOHeader.MeshData[i].MaterialDeclName);
        }

        return meshInfo;
    }

    int ModelConverter::ConvertOBJtoLWO(fs::path gamePath, fs::path inputOBJ, fs::path targetLWO, fs::path resourcePath, std::string material2decl, bool useYOrientation)
    {
        fs::path basePath = gamePath / "base";

        // Make sure we have Oodle DLL available
        if (!oodleInit(basePath.string()))
            return 0;

        // Load the original OBJ data into memory
        OBJFile inputOBJData(inputOBJ);

        // Use temporary files to avoid overwriting originals
        fs::path tmpOBJFile = inputOBJ;
        fs::path tmpMTLFile = inputOBJ;
        tmpOBJFile.replace_extension(".obj.tmp");
        tmpMTLFile.replace_extension(".mtl.tmp");

        // Write the original OBJ data to our temporary file
        // We aren't using fs::copy_file because we *do not* want an identical copy.
        // Our OBJ constructor also sanitizes the file. We need this sanitized version to avoid assertion errors in vendor/obj.
        std::ofstream outfile(tmpOBJFile);
        if (outfile.is_open())
        {
            for (int i = 0; i < inputOBJData.Lines.size(); i++)
            {
                std::string line = inputOBJData.Lines[i].lineData;
                outfile << line << "\n";
            }
        }
        outfile.close();

        // Now use vendor/obj tool on our "sanitized" OBJ file
        // This tool will reindex the OBJ file verts/faces to make it OpenGL/Vulkan compatible
        obj* objFile = obj_create(tmpOBJFile.string().c_str());
        obj_write(objFile, tmpOBJFile.string().c_str(), tmpMTLFile.string().c_str(), 8); // precision 8

        // Read the Vulkan compatible OBJ back into memory
        OBJFile objVulkan(tmpOBJFile);

        // Construct LWO geometry from our OBJ data
        // This is an intermediate format for ease of use, still needs to be processed & packed into game format
        LWO_GEO_UNPACKED lwoGeo(objVulkan, useYOrientation);

        // Error check: DOOM Eternal supports maximum 65535 vertices per mesh.
        // If the Vulkan compatible OBJ file has too many vertices, we need to abort. 
        // Vulkan compatible OBJ may require 3-5x as many vertices as the original OBJ file.
        if (lwoGeo.Vertices.size() > 65535)
        {
            // Delete the temporary files
            fs::remove(tmpOBJFile);
            fs::remove(tmpMTLFile);

            // Set vert count for error message and return
            VertexCount = lwoGeo.Vertices.size();
            return 0;
        }

        // Find the required offsets in the unpacked geo
        std::vector<float_t> x;
        std::vector<float_t> y;
        std::vector<float_t> z;
        std::vector<float_t> u;
        std::vector<float_t> v;

        x.resize(lwoGeo.Vertices.size());
        y.resize(lwoGeo.Vertices.size());
        z.resize(lwoGeo.Vertices.size());
        u.resize(lwoGeo.UVs.size());
        v.resize(lwoGeo.UVs.size());

        for (int i = 0; i < x.size(); i++)
            x[i] = lwoGeo.Vertices[i].x;
        for (int i = 0; i < y.size(); i++)
            y[i] = lwoGeo.Vertices[i].y;
        for (int i = 0; i < z.size(); i++)
            z[i] = lwoGeo.Vertices[i].z;
        for (int i = 0; i < u.size(); i++)
            u[i] = lwoGeo.UVs[i].u;
        for (int i = 0; i < v.size(); i++)
            v[i] = -(lwoGeo.UVs[i].v) + 1;

        float_t minX = *std::min_element(x.begin(), x.end());
        float_t minY = *std::min_element(y.begin(), y.end());
        float_t minZ = *std::min_element(z.begin(), z.end());

        float_t maxX = *std::max_element(x.begin(), x.end());
        float_t maxY = *std::max_element(y.begin(), y.end());
        float_t maxZ = *std::max_element(z.begin(), z.end());

        float_t minU = *std::min_element(u.begin(), u.end());
        float_t minV = *std::min_element(v.begin(), v.end());

        float_t diffX = maxX - minX;
        float_t diffY = maxY - minY;
        float_t diffZ = maxZ - minZ;

        std::vector<float_t> diffXYZ = { diffX, diffY, diffZ };
        float_t scale = *std::max_element(diffXYZ.begin(), diffXYZ.end());

        // Pack Geometry into LWO format
        LWO_GEO_PACKED lwoGeoPacked;
        lwoGeoPacked.PackGeometry(lwoGeo, minX, minY, minZ, minU, minV, scale);

        // Get the hashID for this file in .streamdb
        ResourceFileReader resourceFileReader(resourcePath);
        uint64_t resourceIndex = resourceFileReader.GetResourceIndex(targetLWO);
        endianSwap(resourceIndex);
        uint64_t streamDBIndex = resourceFileReader.CalculateStreamDBIndex(resourceIndex, -6);
        endianSwap(streamDBIndex);

        // create import and streamdb folders
        fs::path importPath = "imports";
        
        std::string lwoFileNameForImportPath = targetLWO.filename().replace_extension("").string();
        std::string indexStringForImportPath = std::to_string(streamDBIndex);

        fs::path thisImportPath = importPath / fs::path(lwoFileNameForImportPath + "_id#" + indexStringForImportPath);
        fs::path streamdbPath =  thisImportPath / fs::path("streamdb");
        
        if (!fs::exists(streamdbPath))
            if (!mkpath(streamdbPath))
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", streamdbPath.string().c_str());

        // create model path within streamdb directory
        fs::path lwoPathNoFilename = targetLWO;
        fs::path modelPath = streamdbPath / lwoPathNoFilename.remove_filename();
        modelPath.make_preferred();

        if (!fs::exists(modelPath))
            if (!mkpath(modelPath))
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", modelPath.string().c_str());

        // Write to .lwo binary file
        fs::path modelBody = targetLWO.filename().replace_extension("");
        modelBody = modelPath / modelBody;
        std::string modelBodyStr = modelBody.string() + "_id#" + std::to_string(streamDBIndex) + ".lwo";

        uint64_t decompressedSize = 0;
        fs::path modelBodyPathWideStr = fs::current_path() / fs::path(modelBodyStr);

        FILE* f = openLongFilePath(modelBodyPathWideStr); //wb

        if (f != NULL)
        {
            for (int i = 0; i < lwoGeoPacked.Vertices.size(); i++)
                fwrite(&lwoGeoPacked.Vertices[i], sizeof(LWO_VERTEX_PACKED), 1, f);

            for (int i = 0; i < lwoGeoPacked.Normals.size(); i++)
                fwrite(&lwoGeoPacked.Normals[i], sizeof(LWO_NORMAL_PACKED), 1, f);

            for (int i = 0; i < lwoGeoPacked.UVs.size(); i++)
                fwrite(&lwoGeoPacked.UVs[i], sizeof(LWO_UV_PACKED), 1, f);

            for (int i = 0; i < lwoGeoPacked.Colors.size(); i++)
                fwrite(&lwoGeoPacked.Colors[i], sizeof(LWO_COLORS), 1, f);

            for (int i = 0; i < lwoGeoPacked.Faces.size(); i++)
                fwrite(&lwoGeoPacked.Faces[i], sizeof(LWO_FACE_GROUP), 1, f);

            decompressedSize = ftell(f);
            fclose(f);
        }

        // Compress our geometry file for injection
        std::string compressedGeometry = modelBodyStr;
        int compressedSize = oodleCompress(modelBodyStr, compressedGeometry);

        if (compressedSize <= 0)
        {
            fprintf(stderr, "Error: failed to compress with Oodle DLL.\n");
            return 0;
        }

        // Open up the .lwo header and modify it
        fs::path localLWOPath = ExtractLWOHeader(targetLWO, resourcePath, 0, indexStringForImportPath);

        LWO LWOHeader;
        LWOHeader.Serialize(localLWOPath);

        // Set lod0 header decompressed size (can we remove this?)
        LWOHeader.LWOStreamDBHeaders[0].decompressedSize = decompressedSize;

        // Change mesh count to 1 no matter what
        LWOHeader.Header.NumMeshes = 1;

        // Edit the first mesh to use our chosen material2 decl
        LWOHeader.MeshData[0].MeshHeader.DeclStrlen = material2decl.length();
        LWOHeader.MeshData[0].MaterialDeclName.clear(); 
        LWOHeader.MeshData[0].MaterialDeclName.resize(0);

        const char* ptr = material2decl.c_str();
        LWOHeader.MeshData[0].MaterialDeclName.append(ptr);

        // Make sure we have 3 BMLs no matter what
        if (LWOHeader.MeshData[0].BMLHeaders.size() == 1)
        {
            LWOHeader.MeshData[0].BMLHeaders.resize(3);
            LWOHeader.MeshData[0].BMLHeaders[1].UnkFloat1 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat1;
            LWOHeader.MeshData[0].BMLHeaders[1].UnkFloat2 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat2;
            LWOHeader.MeshData[0].BMLHeaders[1].UnkFloat3 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat3;
            LWOHeader.MeshData[0].BMLHeaders[2].UnkFloat1 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat1;
            LWOHeader.MeshData[0].BMLHeaders[2].UnkFloat2 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat2;
            LWOHeader.MeshData[0].BMLHeaders[2].UnkFloat3 = LWOHeader.MeshData[0].BMLHeaders[0].UnkFloat3;

            // set signature for 2nd BML header { "B", "M", "L", "r" }
            LWOHeader.MeshData[0].BMLHeaders[1].signature[0] = 66;
            LWOHeader.MeshData[0].BMLHeaders[1].signature[1] = 77;
            LWOHeader.MeshData[0].BMLHeaders[1].signature[2] = 76;
            LWOHeader.MeshData[0].BMLHeaders[1].signature[3] = 114;
            
            // set signature for 3rd BML header { "B", "M", "L", "r" }
            LWOHeader.MeshData[0].BMLHeaders[2].signature[0] = 66;  
            LWOHeader.MeshData[0].BMLHeaders[2].signature[1] = 77;
            LWOHeader.MeshData[0].BMLHeaders[2].signature[2] = 76;
            LWOHeader.MeshData[0].BMLHeaders[2].signature[3] = 114;
        }

        // Make all 3 LODs identical for now - we only use first mesh
        for (int i = 0; i < LWOHeader.MeshData[0].BMLHeaders.size(); i++)
        {
            LWO_BML_HEADER& BMLHeader = LWOHeader.MeshData[0].BMLHeaders[i];

            BMLHeader.NumVertices = lwoGeoPacked.Vertices.size();
            BMLHeader.NumFacesX3 = lwoGeoPacked.Faces.size() * 3;

            BMLHeader.NegBoundsX = minX;
            BMLHeader.NegBoundsY = minY;
            BMLHeader.NegBoundsZ = minZ;
            BMLHeader.PosBoundsX = maxX;
            BMLHeader.PosBoundsY = maxY;
            BMLHeader.PosBoundsZ = maxZ;

            BMLHeader.VertexOffsetX = minX;
            BMLHeader.VertexOffsetY = minY;
            BMLHeader.VertexOffsetZ = minZ;
            BMLHeader.UVMapOffsetU = minU;
            BMLHeader.UVMapOffsetV = minV;

            BMLHeader.VertexScale = scale;
            BMLHeader.UVScale = 1;

            // Change LWO version to 60 and 2
            BMLHeader.LWOVersion = 60;
            BMLHeader.LWOVersion2 = 2;
        }

        // Discard original LWO settings - these must be zero for a single-mesh LWO or game will crash
        LWOHeader.LWOSettings2.boolCompressVertexStreams = 0;
        LWOHeader.LWOSettings2.boolUseMultiLayer = 0;

        // NumOffsets == 5 means we have UV lightmap, alternate format is used
        // Copy this to the standard format. Our custom LWO discards any UV lightmap info
        if (LWOHeader.LWOStreamDBHeaders[0].NumOffsets == 5)
        {
            for (int i = 0; i < 5; i++)
            {
                LWOHeader.LWOStreamDBData[i].unkNormalInt = LWOHeader.LWOStreamDBData_124[i].unkNormalInt;
                LWOHeader.LWOStreamDBData[i].unkUVInt = LWOHeader.LWOStreamDBData_124[i].unkUVInt;
                LWOHeader.LWOStreamDBData[i].unkColorInt = LWOHeader.LWOStreamDBData_124[i].unkColorInt;
                LWOHeader.LWOStreamDBData[i].unkFacesInt = LWOHeader.LWOStreamDBData_124[i].unkFacesInt;
                LWOHeader.LWOStreamDBData[i].unkInt99 = LWOHeader.LWOStreamDBData_124[i].unkInt99;
            }
        }

        // Change all the streamdb data to use version 60 / 2 - no uv lightmap
        for (int i = 0; i < 5; i++)
        {
            LWOHeader.LWOStreamDBHeaders[i].LWOVersion = 60;
            LWOHeader.LWOStreamDBHeaders[i].LWOVersion2 = 2;
            LWOHeader.LWOStreamDBHeaders[i].NumOffsets = 4;
        }

        // Make first 3 streamdb data identical since we are duplicating the highest LOD, last 2 aren't used.
        for (int i = 0; i < 3; i++)
        {
            LWOHeader.LWOStreamDBData[i].LOD_NormalStartOffset = lwoGeoPacked.Vertices.size() * 8;
            LWOHeader.LWOStreamDBData[i].LOD_UVStartOffset = lwoGeoPacked.Vertices.size() * 16;
            LWOHeader.LWOStreamDBData[i].LOD_ColorStartOffset = lwoGeoPacked.Vertices.size() * 20;
            LWOHeader.LWOStreamDBData[i].LOD_FacesStartOffset = lwoGeoPacked.Vertices.size() * 24;
            LWOHeader.LWOGeoStreamDiskLayout[i].StreamCompressionType = 4;
            LWOHeader.LWOGeoStreamDiskLayout[i].decompressedSize = decompressedSize;
            LWOHeader.LWOGeoStreamDiskLayout[i].compressedSize = compressedSize;
        }

        // Calculate cumulative streamdb sizes
        LWOHeader.LWOGeoStreamDiskLayout[1].cumulativeStreamDBCompSize = LWOHeader.LWOGeoStreamDiskLayout[0].compressedSize;
        LWOHeader.LWOGeoStreamDiskLayout[2].cumulativeStreamDBCompSize = LWOHeader.LWOGeoStreamDiskLayout[1].compressedSize + LWOHeader.LWOGeoStreamDiskLayout[1].cumulativeStreamDBCompSize;
        LWOHeader.LWOGeoStreamDiskLayout[3].cumulativeStreamDBCompSize = LWOHeader.LWOGeoStreamDiskLayout[2].compressedSize + LWOHeader.LWOGeoStreamDiskLayout[2].cumulativeStreamDBCompSize;
        LWOHeader.LWOGeoStreamDiskLayout[4].cumulativeStreamDBCompSize = LWOHeader.LWOGeoStreamDiskLayout[3].compressedSize + LWOHeader.LWOGeoStreamDiskLayout[3].cumulativeStreamDBCompSize;

        // Make streamdb header decompressed sizes the same
        LWOHeader.LWOStreamDBHeaders[1].decompressedSize = decompressedSize;
        LWOHeader.LWOStreamDBHeaders[2].decompressedSize = decompressedSize;

        // Open lwo header for writing
        std::string lwoHeaderFile = localLWOPath.string();

        fs::path lwoHeaderPathWide = fs::current_path() / fs::path(lwoHeaderFile);
        FILE* fw = openLongFilePath(lwoHeaderPathWide); //wb
        if (fw != NULL)
        {
            fwrite(&LWOHeader.Header, sizeof(LWO_HEADER), 1, fw);

            // Write Mesh metadata - use 1st mesh only
            fwrite(&LWOHeader.MeshData[0].MeshHeader, sizeof(LWO_MESH_HEADER), 1, fw);
            fwrite(&LWOHeader.MeshData[0].MaterialDeclName[0], 1, LWOHeader.MeshData[0].MaterialDeclName.length(), fw);
            fwrite(&LWOHeader.MeshData[0].MeshFooter, sizeof(LWO_MESH_FOOTER), 1, fw);

            // Write BMLr metadata
            for (int i = 0; i < LWOHeader.MeshData[0].BMLHeaders.size(); i++)
            {
                fwrite(&LWOHeader.MeshData[0].BMLHeaders[i], sizeof(LWO_BML_HEADER), 1, fw);
            }

            // Write LWO Settings
            fwrite(&LWOHeader.LWOSettings, sizeof(LWO_SETTINGS), 1, fw);

            // Just set this to zero - we don't know what it does.
            // If it is "1" there is usually some extra data afterwards, but we aren't going to write this in our custom LWO.
            LWOHeader.LWOSettings.unkFlag1 = 0;

            // We dont know what these chunks are, write zero here so the game doesn't expect them.
            LWOHeader.Num32ByteChunks = 0;
            fwrite(&LWOHeader.Num32ByteChunks, sizeof(uint32_t), 1, fw);

            // Write unk mesh strlen
            fwrite(&LWOHeader.MeshStrlen, sizeof(uint32_t), 1, fw);
            if (LWOHeader.MeshStrlen > 0)
            {
                fwrite(&LWOHeader.MeshName[0], 1, LWOHeader.MeshName.length(), fw);
            }
        
            // Write LWO Settings 2
            fwrite(&LWOHeader.LWOSettings2, sizeof(LWO_SETTINGS_2), 1, fw);

            // Write StreamDB Data
            for (int i = 0; i < 5; i++)
            {
                fwrite(&LWOHeader.LWOStreamDBHeaders[i], sizeof(LWO_STREAMDB_HEADER), 1, fw);
                fwrite(&LWOHeader.LWOStreamDBData[i], sizeof(LWO_STREAMDB_DATA), 1, fw);
                fwrite(&LWOHeader.LWOGeoStreamDiskLayout[i], sizeof(LWO_GEOMETRY_STREAMDISK_LAYOUT), 1, fw);
            }

            // Finish and close file
            fclose(fw);
        }

        // Remove the temporary OBJ/MTL files
        fs::remove(tmpOBJFile);
        fs::remove(tmpMTLFile);

        return 1;
    }
};
