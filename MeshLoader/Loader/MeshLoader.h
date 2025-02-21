#pragma once 
#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#ifdef DEBUG
#pragma comment(lib, "/External/lib/debug/assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "/External/lib/release/assimp-vc143-mtd.lib")
#endif // DEBUG
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


class MeshLoader {
public:
	MeshLoader() = default;
	~MeshLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path);
private:
	Assimp::Importer mImporter{};
};

