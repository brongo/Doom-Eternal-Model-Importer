#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cmath>

#include "OBJ.h"

#pragma pack(push)	// Not portable, sorry.
#pragma pack(1)		// Works on my machine (TM).

namespace fs = std::filesystem;

namespace HAYDEN
{
	struct LWO_VERTEX_PACKED
	{
		uint16_t x = 0;
		uint16_t y = 0;
		uint16_t z = 0;
		uint16_t nullPad = 0;
	};

	struct LWO_VERTEX
	{	
		float_t x = 0;
		float_t y = 0;
		float_t z = 0;
	};

	struct LWO_NORMAL
	{
		float_t xn = 0;
		float_t yn = 0;
		float_t zn = 0;
	};

	struct LWO_NORMAL_PACKED
	{
		uint8_t xn = 0;
		uint8_t yn = 0;
		uint8_t zn = 0;
		uint8_t always0 = 0;
		uint8_t xt = 0;
		uint8_t yt = 0;
		uint8_t zt = 0;
		uint8_t always128 = 128;
	};

	struct LWO_UV
	{
		float_t u = 0;
		float_t v = 0;
	};
	struct LWO_UV_PACKED
	{
		uint16_t u = 0;
		uint16_t v = 0;
	};

	struct LWO_FACE_GROUP
	{
		uint16_t f1 = 0;
		uint16_t f2 = 0;
		uint16_t f3 = 0;
	};

	struct LWO_COLORS
	{
		uint8_t r = 153;
		uint8_t g = 153;
		uint8_t b = 153;
		uint8_t a = 255;
	};

	struct LWO_HEADER
	{
		uint64_t NullPad64_0 = 0;
		uint32_t NullPad32_0 = 0;
		uint32_t FileType = 0;
		uint32_t NumMeshes = 0;
		uint32_t NullPad32_1 = 0;
		uint64_t UnkHash = 0;				// if zero, uses alternate BML type - LWO_BML_HEADER_ALT - possibly means not streamed?
		uint32_t NullPad32_2 = 0;
	};

	struct LWO_MESH_HEADER
	{
		uint32_t UnkInt1 = 0;
		uint32_t UnkInt2 = 0;
		uint32_t DeclStrlen = 0;
	};

	struct LWO_MESH_FOOTER
	{
		uint32_t UnkInt3 = 0;
		uint32_t Dummy1 = 0;
		uint32_t NullPad = 0;
	};

	struct LWO_BML_HEADER
	{
		uint32_t NullPad32 = 0;
		uint32_t DummyMask = 4294967294;
		uint32_t NumVertices = 0;
		uint32_t NumFacesX3 = 0;
		uint16_t LWOVersion = 0;				// indicates model type. 60 is standard
		uint16_t LWOVersion2 = 0;				// always 2?

		float_t NegBoundsX = 0;
		float_t NegBoundsY = 0;
		float_t NegBoundsZ = 0;
		float_t PosBoundsX = 0;
		float_t PosBoundsY = 0;
		float_t PosBoundsZ = 0;

		float_t VertexOffsetX = 0;
		float_t VertexOffsetY = 0;
		float_t VertexOffsetZ = 0;
		float_t VertexScale = 0;

		float_t UVMapOffsetU = 0;
		float_t UVMapOffsetV = 0;
		float_t UVScale = 0;

		float_t UnkFloat1 = 0;
		float_t UnkFloat2 = 0;
		float_t UnkFloat3 = 0;
		char signature[4] = { 0 };
	};

	struct LWO_MESH_DATA
	{
		LWO_MESH_HEADER MeshHeader;
		std::string MaterialDeclName;
		LWO_MESH_FOOTER MeshFooter;
		std::vector<LWO_BML_HEADER> BMLHeaders;

		// Only if LWO_HEADER.UnkHash == 0
		uint32_t unkTuple[2] = { 0 };
	};

	struct LWO_SETTINGS
	{
		uint32_t unkInt6 = 0;
		uint32_t unkInt7 = 0;
		uint32_t unkFileID = 0;		// if this != 0, there's a new section after LWO Settings
		uint32_t unkInt8 = 0;		// 0xFFFF if unkFileID = 0
		uint32_t unkInt9 = 0;		// 0xFFFF if unkFileID = 0
		uint32_t unkFlag1 = 0;
	};

	struct UNK_32_CHUNK
	{
		float_t unknowns[8] = { 0 };
	};

	struct LWO_SETTINGS_2
	{
		char   unkBool_0_1 = 0;
		char   unkBool_0_2 = 1;
		char   unkBool_0_3 = 0;
		char   boolTangentBumpDummy = 0;
		char   boolRenderBump = 1;
		char   boolGenerateUnwrap = 1;
		char   boolSurfaceHasLightmapUVs = 1;
		char   boolCompressVertexStreams = 0;
		char   boolUseMultiLayer = 0;
	};

	struct LWO_STREAMDB_HEADER
	{
		uint32_t NumStreams = 1;				
		uint16_t LWOVersion = 60;				// 60 is normal, 124 is uvlayout_lightmap = 1
		uint16_t LWOVersion2 = 2;
		uint32_t decompressedSize = 0;
		uint32_t NumOffsets = 4;				// always 4 (or 5 if uvlayout_lightmap = 1)
	};

	struct LWO_STREAMDB_DATA
	{
		uint32_t unkNormalInt = 32;
		uint32_t unkUVInt = 20;
		uint32_t unkColorInt = 131072;
		uint32_t unkFacesInt = 8;
		uint32_t unkInt99 = 0;
		uint32_t LOD_NormalStartOffset = 0;
		uint32_t LOD_UVStartOffset = 0;
		uint32_t LOD_ColorStartOffset = 0;
		uint32_t LOD_FacesStartOffset = 0;
	};

	struct LWO_GEOMETRY_STREAMDISK_LAYOUT
	{
		uint32_t StreamCompressionType = 3;		// 3 = NONE_MODEL, 4 = KRAKEN_MODEL
		uint32_t decompressedSize = 0;
		uint32_t compressedSize = 0;
		uint32_t cumulativeStreamDBCompSize = 0;
	};

	struct LWO_STREAMDB_DATA_VARIANT
	{
		uint32_t unkNormalInt = 32;
		uint32_t unkUVInt = 20;
		uint32_t unkUVLightmapInt = 64;			// 124 variant only
		uint32_t unkColorInt = 131072;
		uint32_t unkFacesInt = 8;					
		uint32_t unkInt99 = 0;
		uint32_t LOD_NormalStartOffset = 0;
		uint32_t LOD_UVStartOffset = 0;
		uint32_t LOD_UVLightMapOffset = 0;		// 124 variant only
		uint32_t LOD_ColorStartOffset = 0;
		uint32_t LOD_FacesStartOffset = 0;
		uint32_t StreamCompressionType = 0;
		uint32_t decompressedSize = 0;
		uint32_t compressedSize = 0;
		uint32_t cumulativeStreamDBCompSize = 0;
	};

	class LWO_GEO_UNPACKED
	{
		public:
			std::vector<LWO_VERTEX> Vertices;
			std::vector<LWO_NORMAL> Normals;
			std::vector<LWO_UV> UVs;
			std::vector<LWO_COLORS> Colors;
			std::vector<LWO_FACE_GROUP> Faces;
			LWO_GEO_UNPACKED (OBJFile objFile, bool useYOrientation);
	};

	class LWO_GEO_PACKED
	{
		public:
			std::vector<LWO_VERTEX_PACKED> Vertices;
			std::vector<LWO_NORMAL_PACKED> Normals;
			std::vector<LWO_UV_PACKED> UVs;
			std::vector<LWO_COLORS> Colors;
			std::vector<LWO_FACE_GROUP> Faces;
			void PackGeometry(LWO_GEO_UNPACKED geometry, float_t minX, float_t minY, float_t minZ, float_t minU, float_t minV, float_t scale);
	};

	class LWO
	{
		public:
			
			// Serialized file data
			LWO_HEADER Header;
			std::vector<LWO_MESH_DATA> MeshData;
			LWO_SETTINGS LWOSettings;
			uint32_t MeshStrlen = 0;
			std::string MeshName;
			LWO_SETTINGS_2 LWOSettings2;
			std::vector<LWO_STREAMDB_HEADER> LWOStreamDBHeaders;

			// Used in LWO version = 60
			std::vector<LWO_STREAMDB_DATA> LWOStreamDBData;

			// Used in LWO version = 124 (uv lightmap)
			std::vector<LWO_STREAMDB_DATA_VARIANT> LWOStreamDBData_124;

			// Always used
			std::vector<LWO_GEOMETRY_STREAMDISK_LAYOUT> LWOGeoStreamDiskLayout;

			// Variants only
			std::vector<UNK_32_CHUNK> UnkChunkData;
			std::vector<float_t> UnkVariantFloats;
			
			// Defaults
			int  bmlCount = 3;
			bool useExtendedBML = 0;
			uint32_t Num32ByteChunks = 0;

			// Constructor
			void Serialize(fs::path modelPath);
	};
}

#pragma pack(pop)