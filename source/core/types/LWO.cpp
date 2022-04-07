#include "LWO.h"

namespace HAYDEN
{
    // Get unpacked geometry from OBJ file
    LWO_GEO_UNPACKED::LWO_GEO_UNPACKED(OBJFile objFile, bool useYOrientation)
    {
        // read verts for each object - store sequentially
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            for (int j = 0; j < objFile.Objects[i].Vertices.size(); j++)
            {
                LWO_VERTEX vertex;
                std::string thisLine = objFile.Objects[i].Vertices[j];
                size_t sep;

                // get z
                sep = thisLine.rfind(" ");
                std::string zString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // get y
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string yString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // get x
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string xString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // populate vert
                if (!useYOrientation)
                {
                    vertex.x = std::stof(xString);
                    vertex.y = std::stof(yString);
                    vertex.z = std::stof(zString);
                }
                else
                {
                    vertex.x = std::stof(xString);
                    vertex.z = std::stof(yString);
                    vertex.y = -(std::stof(zString));
                }

                Vertices.push_back(vertex);
            }
        }

        // read UVs for each object - store sequentially
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            for (int j = 0; j < objFile.Objects[i].UVs.size(); j++)
            {
                LWO_UV uv;
                std::string thisLine = objFile.Objects[i].UVs[j];
                size_t sep;

                // get v
                sep = thisLine.rfind(" ");
                std::string vString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // get u
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string uString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // populate UV
                uv.u = std::stof(uString);
                uv.v = std::stof(vString);

                UVs.push_back(uv);
            }
        }

        // read Normals for each object - store sequentially
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            for (int j = 0; j < objFile.Objects[i].Normals.size(); j++)
            {
                LWO_NORMAL normal;
                std::string thisLine = objFile.Objects[i].Normals[j];
                size_t sep;

                // get z
                sep = thisLine.rfind(" ");
                std::string zString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // get y
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string yString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // get x
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string xString = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));

                // populate normal
                if (!useYOrientation)
                {
                    normal.xn = std::stof(xString);
                    normal.yn = std::stof(yString);
                    normal.zn = std::stof(zString);
                }
                else
                {
                    normal.xn = std::stof(xString);
                    normal.zn = std::stof(yString);
                    normal.yn = -(std::stof(zString));
                }

                Normals.push_back(normal);
            }
        }

        // read Faces for each object / store sequentially
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            for (int j = 0; j < objFile.Objects[i].Faces.size(); j++)
            {
                LWO_FACE_GROUP face;
                std::string thisLine = objFile.Objects[i].Faces[j];
                size_t sep;
                size_t sep2;

                // get f3
                sep = thisLine.rfind(" ");
                std::string f3String = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));
                sep2 = f3String.rfind("/");
                f3String = f3String.substr(sep2 + 1, f3String.length() - (sep2 + 1));

                // get f1
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string f1String = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));
                sep2 = f1String.rfind("/");
                f1String = f1String.substr(sep2 + 1, f1String.length() - (sep2 + 1));

                // get f2
                thisLine = thisLine.substr(0, sep);
                sep = thisLine.rfind(" ");
                std::string f2String = thisLine.substr(sep + 1, thisLine.length() - (sep + 1));
                sep2 = f2String.rfind("/");
                f2String = f2String.substr(sep2 + 1, f2String.length() - (sep2 + 1));

                // populate normal
                face.f3 = std::stoi(f1String) - 1; // 0 indexed
                face.f1 = std::stoi(f2String) - 1;
                face.f2 = std::stoi(f3String) - 1;

                Faces.push_back(face);
            }
        }

        // colors
        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            for (int j = 0; j < objFile.Objects[i].Vertices.size(); j++)
            {
                LWO_COLORS colorsTmp;
                Colors.push_back(colorsTmp);
            }
        }

        return;
    }

    void LWO_GEO_PACKED::PackGeometry(LWO_GEO_UNPACKED geo, float_t minX, float_t minY, float_t minZ, float_t minU, float_t minV, float_t scale)
    {
        for (int i = 0; i < geo.Vertices.size(); i++)
        {
            float offsetX = minX;
            float offsetY = minY;
            float offsetZ = minZ;

            LWO_VERTEX_PACKED packedVertex;
            packedVertex.x = round(((geo.Vertices[i].x - offsetX) / scale) * 65535);
            packedVertex.y = round(((geo.Vertices[i].y - offsetY) / scale) * 65535);
            packedVertex.z = round(((geo.Vertices[i].z - offsetZ) / scale) * 65535);

            Vertices.push_back(packedVertex);
        }

        for (int i = 0; i < geo.UVs.size(); i++)
        {
            float offsetU = minU;
            float offsetV = minV;

            LWO_UV_PACKED packedUV;
            packedUV.u = round((geo.UVs[i].u - offsetU) * 65535);
            packedUV.v = round((-(geo.UVs[i].v) + (1 - offsetV)) * 65535);

            UVs.push_back(packedUV);
        }

        for (int i = 0; i < geo.Normals.size(); i++)
        {
            LWO_NORMAL_PACKED packedNormal;

            packedNormal.xn = round(((geo.Normals[i].xn + 1) / 2) * 255);
            packedNormal.yn = round(((geo.Normals[i].yn + 1) / 2) * 255);
            packedNormal.zn = round(((geo.Normals[i].zn + 1) / 2) * 255);

            Normals.push_back(packedNormal);
        }

        Faces = geo.Faces;
        Colors = geo.Colors;

        return;
    };

    void LWO::Serialize(fs::path modelPath)
    {
        std::string modelPathStr = modelPath.string();
        
        FILE* f = fopen(modelPathStr.c_str(), "rb");
        if (f != NULL)
        {
            // Read number of meshes
            fread(&Header, sizeof(LWO_HEADER), 1, f);
            MeshData.resize(Header.NumMeshes);

            // Determine BMLr type & count
            if (Header.UnkHash == 0)
            {
                this->bmlCount = 1;
                this->useExtendedBML = 1;
            }
            else
            {
                this->bmlCount = 3;
                this->useExtendedBML = 0;
            }

            if (Header.NullPad32_2 != 0)
            {
                return;
            }

            // Read mesh and material info
            for (int i = 0; i < Header.NumMeshes; i++)
            {
                fread(&MeshData[i].MeshHeader, sizeof(LWO_MESH_HEADER), 1, f);

                int strLen = MeshData[i].MeshHeader.DeclStrlen;

                if (strLen < 0 || strLen > 1024)
                {
                    return;
                }

                MeshData[i].MaterialDeclName.resize(strLen);

                fread(&MeshData[i].MaterialDeclName[0], strLen, 1, f);
                fread(&MeshData[i].MeshFooter, sizeof(LWO_MESH_FOOTER), 1, f);

                // Read BMLr data (level-of-detail info for each mesh)
                MeshData[i].BMLHeaders.resize(bmlCount);
                
                for (int j = 0; j < bmlCount; j++)
                {
                    fread(&MeshData[i].BMLHeaders[j], sizeof(LWO_BML_HEADER), 1, f);

                    // No idea what these are for
                    if (this->useExtendedBML)
                    {
                        fread(&MeshData[i].unkTuple[0], sizeof(uint32_t), 1, f);
                        fread(&MeshData[i].unkTuple[1], sizeof(uint32_t), 1, f);
                    }
                }
            }

            // Read LWO Settings
            fread(&LWOSettings, sizeof(LWO_SETTINGS), 1, f);

            // Fill in defaults, our custom LWO models won't use these, but some native models in the game do.
            MeshStrlen = 0;
            Num32ByteChunks = 0;

            // There will always be 5 of these.
            LWOStreamDBHeaders.resize(5);
            LWOStreamDBData.resize(5);
            LWOGeoStreamDiskLayout.resize(5);

            fclose(f);
        }       
        return;
    }
}
