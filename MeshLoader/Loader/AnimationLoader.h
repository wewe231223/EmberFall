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
#include "../MeshLoader/Base/AnimationData.h"

class AnimationLoader {
public:
	AnimationLoader() = default;
	~AnimationLoader() = default;
public:
	AnimationClip Load(const std::filesystem::path& path, UINT animIndex = 0);
private:
	std::shared_ptr<BoneNode> buildHierarchy(const aiNode* node);
private:
	Assimp::Importer mImporter{};
};