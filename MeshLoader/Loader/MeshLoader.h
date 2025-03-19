#pragma once 
#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "External/lib/release/assimp-vc143-mt.lib")
#endif // DEBUG
#include "../External/Include/assimp/Importer.hpp"
#include "../External/Include/assimp/scene.h"
#include "../External/Include/assimp/postprocess.h"
#ifdef max 
#undef max 
#endif 

class MeshLoader {
public:
	MeshLoader() = default;
	~MeshLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path, UINT meshIndex = std::numeric_limits<UINT>::max());
private:
	static Assimp::Importer mImporter;
};

