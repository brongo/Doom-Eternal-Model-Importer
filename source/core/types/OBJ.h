#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <locale> // std::isalpha

namespace fs = std::filesystem;

namespace HAYDEN
{
	enum class OBJLineType
	{
		DEFAULT = 0,
		MTLLIB = 1,
		OBJECT = 2,
		VERTEX = 3,
		UV = 4,
		NORMAL = 5,
		FACE = 6,
		G = 7,
		USEMTL = 8,
		COMMENT = 9
	};

	struct OBJSingleLine
	{
		OBJLineType lineType = OBJLineType::DEFAULT;
		std::string lineData;
	};

	struct OBJFile_Object
	{
		int StartLine = 0;
		int EndLine = 0;

		std::string ObjectName;
		std::vector<std::string> Vertices;
		std::vector<std::string> Normals;
		std::vector<std::string> UVs;
		std::string GLine;
		std::string UseMaterialLine;
		std::vector<std::string> Faces;
		std::vector<OBJSingleLine> OBJLineData;
	};

	class OBJFile
	{
		public:
			std::string SignatureLine;
			std::string MaterialLine;
			std::vector<OBJFile_Object> Objects;
			std::vector<OBJSingleLine> Lines;
			OBJFile(fs::path modelPath);
	};
}
