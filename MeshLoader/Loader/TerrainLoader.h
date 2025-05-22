#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#include "../Utility/DirectXInclude.h"
#include "../Utility/Defines.h"
#include <vector>

class TerrainLoader {
    static constexpr int PATCH_LENGTH = 4;
    static constexpr int PATCH_SCALE = 8;
    static constexpr int TILE_SCALE = 15;

public:
    TerrainLoader() = default;
    TerrainLoader(const std::filesystem::path& path); 
    
    ~TerrainLoader() = default;

    void Load(const std::filesystem::path& path);

    MeshData GetData() const;
    MeshData GetData(int patchRow, int patchCol) const;

    int GetLength() const { return mLength; }

    const std::pair<int, int> GetPatchCount() const;
private:
    SimpleMath::Vector3 CalculateNormal(int z, int x) const;
    void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) const;

    std::vector<std::vector<float>> mHeight{};
    int mLength = 0;
};

class TerrainCollider {
public:
	TerrainCollider() = default;
	~TerrainCollider() = default;
public:
	bool LoadFromFile(const std::filesystem::path& filePath);
	float GetHeight(float x, float z) const;

	TerrainHeader& GetHeader();
	std::vector<SimpleMath::Vector3>& GetData();
private:
	std::vector<SimpleMath::Vector3> mGlobalVertices;
	TerrainHeader mHeader{}; 
};