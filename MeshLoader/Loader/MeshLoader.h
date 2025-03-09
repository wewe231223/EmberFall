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


class MeshLoader {
public:
	MeshLoader() = default;
	~MeshLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path);
private:
	static Assimp::Importer mImporter;
};

