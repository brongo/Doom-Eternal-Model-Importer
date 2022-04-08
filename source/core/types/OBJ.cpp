#include "OBJ.h"

namespace HAYDEN
{
    OBJFile::OBJFile(fs::path modelPath)
    {
        std::ifstream modelFile(modelPath);
        if (modelFile.is_open())
        {
            std::stringstream inputStream;
            inputStream << modelFile.rdbuf();

            std::string line;
            std::vector<std::string> lines;
            size_t splitPos = 0;

            while (std::getline(inputStream, line))
            {
                // find the first empty space
                if (line.length() <= 1)
                    continue;

                OBJSingleLine thisLine;
                splitPos = line.find(" ");

                // skip empty lines
                if (splitPos == -1)
                    continue;

                std::string lineStart = line.substr(0, splitPos);
                thisLine.lineType = OBJLineType::DEFAULT;

                if (lineStart == "#")
                    thisLine.lineType = OBJLineType::COMMENT;

                if (lineStart == "mtllib")
                    thisLine.lineType = OBJLineType::MTLLIB;

                if (lineStart == "o")
                    thisLine.lineType = OBJLineType::OBJECT;

                if (lineStart == "v")
                    thisLine.lineType = OBJLineType::VERTEX;

                if (lineStart == "vt")
                    thisLine.lineType = OBJLineType::UV;

                if (lineStart == "vn")
                    thisLine.lineType = OBJLineType::NORMAL;

                if (lineStart == "f")
                    thisLine.lineType = OBJLineType::FACE;

                if (lineStart == "g")
                    thisLine.lineType = OBJLineType::G;

                if (lineStart == "usemtl")
                    thisLine.lineType = OBJLineType::USEMTL;

                // skip smooth groups - causes assertion error in vendor/obj
                if (lineStart == "s")
                    continue;

                // skip anything else we don't recognize
                if (thisLine.lineType == OBJLineType::DEFAULT)
                    continue;

                thisLine.lineData = std::string(line);
                Lines.push_back(thisLine);
            }
        }
        modelFile.close();

        // Find all objects
        for (int i = 0; i < Lines.size(); i++)
        {
            if (Lines[i].lineType == OBJLineType::OBJECT)
            {
                OBJFile_Object newObject;
                newObject.StartLine = i + 1;
                newObject.ObjectName = Lines[i].lineData.substr(2, Lines[i].lineData.length());
                Objects.push_back(newObject);

                // Set previous object end line
                if (Objects.size() >= 2)
                {
                    int64_t numObjects = Objects.size();
                    Objects[numObjects - 2].EndLine = i;
                }
            }
        }

        // If no objects are defined, create one called "Default"
        if (Objects.size() == 0)
        {
            OBJFile_Object newObject;
            newObject.StartLine = 1;
            newObject.ObjectName = "Default";
            Objects.push_back(newObject);
        }

        // Set last object endline
        int64_t numObjects = Objects.size();
        Objects[numObjects - 1].EndLine = Lines.size();

        // Split all lines into their respective objects
        for (int i = 0; i < numObjects; i++)
        {
            for (int j = Objects[i].StartLine; j < Objects[i].EndLine; j++)
                Objects[i].OBJLineData.push_back(Lines[j]);
        }

        // Read line data into each object
        for (int i = 0; i < numObjects; i++)
        {
            for (int j = 0; j < Objects[i].OBJLineData.size(); j++)
            {
                OBJLineType thisType = Objects[i].OBJLineData[j].lineType;
                std::string thisLine = Objects[i].OBJLineData[j].lineData;

                switch (thisType)
                {
                    case OBJLineType::VERTEX:
                        Objects[i].Vertices.push_back(thisLine);
                        break;
                    case OBJLineType::UV:
                        Objects[i].UVs.push_back(thisLine);
                        break;
                    case OBJLineType::NORMAL:
                        Objects[i].Normals.push_back(thisLine);
                        break;
                    case OBJLineType::FACE:
                        Objects[i].Faces.push_back(thisLine);
                        break;
                    case OBJLineType::USEMTL:
                        Objects[i].UseMaterialLine = thisLine;
                        break;
                    case OBJLineType::G:
                        Objects[i].GLine = thisLine;
                        break;
                    default:
                        break;
                }
            }    
        }
    }
}
