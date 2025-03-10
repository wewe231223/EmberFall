#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"

class TerrainLoader {
	static constexpr int PATCH_LENGTH = 4;
	static constexpr int PATCH_SCALE = 8;
public:
	TerrainLoader() = default;
	~TerrainLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path, bool patch);
private:
	void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd);
private:
	std::vector<std::vector<float>> mHeight{};
	int mLength{};
};
