#pragma once 
#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"


#ifndef EXCLUDE_ASSIMP
#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "External/lib/release/assimp-vc143-mt.lib")
#endif // DEBUG
#include "../External/Include/assimp/Importer.hpp"
#include "../External/Include/assimp/scene.h"
#include "../External/Include/assimp/postprocess.h"
#endif 

#ifdef max 
#undef max 
#endif 

class MeshLoader {
	static inline const std::filesystem::path BinaryDirectory = "Resources/Assets/MeshBinary";
public:
	MeshLoader() = default;
	~MeshLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path, UINT meshIndex = std::numeric_limits<UINT>::max());
private:
	bool LoadFromBinary(const std::filesystem::path& binaryPath, MeshData& outMeshData);
	bool SaveToBinary(const MeshData& meshData, const std::filesystem::path& binaryPath);
private:
#ifndef EXCLUDE_ASSIMP
	static Assimp::Importer mImporter;
#endif 
};

