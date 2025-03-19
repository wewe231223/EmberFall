#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"

#ifdef max 
#undef max
#endif // max

MeshData TerrainLoader::Load(const std::filesystem::path& path, bool patch) {
    MeshData meshData{};

    CrashExp((path.extension() == ".raw"), "Height map should be .raw File");

    if (patch) {
        size_t size = std::filesystem::file_size(path);
        mLength = static_cast<int>(::sqrt(size));
        int half = mLength / 2;

        std::ifstream file{ path, std::ios::binary };

        mHeight.resize(mLength, std::vector<float>(mLength));
        std::vector<BYTE> data(mLength);

        for (auto& line : mHeight) {
            file.read(reinterpret_cast<char*>(data.data()), mLength * sizeof(BYTE));
            for (size_t i = 0; auto & dot : line) {
                dot = static_cast<float>(data[i++]);
            }
        }

        int patchSize = PATCH_LENGTH * PATCH_SCALE;
        for (int pz = mLength - patchSize; pz >= 0; pz -= patchSize) {
            for (int px = 0; px <= mLength - patchSize; px += patchSize) {
                CreatePatch(meshData, pz + patchSize, pz, px, px + patchSize);
            }
        }

        meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
        meshData.indexed = false;
        meshData.unitCount = static_cast<UINT>(meshData.position.size());
        meshData.vertexAttribute.set(0);
        meshData.vertexAttribute.set(2);
        meshData.vertexAttribute.set(3);

    }
    else {
        // 일반 처리 (변경 없음)
    }

    return meshData;
}



void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) {

    float stepSize = static_cast<float>(zStart - zEnd) / PATCH_LENGTH;
    
    for (int i = 0; i <= PATCH_LENGTH; ++i) {
        for (int j = 0; j <= PATCH_LENGTH; ++j) {
            int z = std::clamp(static_cast<int>(zStart - i * stepSize), 0, mLength - 1);
            int x = std::clamp(static_cast<int>(xStart + j * stepSize), 0, mLength - 1);

            const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1) };
            const DirectX::XMFLOAT2 uv1{ static_cast<float>(j) / PATCH_LENGTH, static_cast<float>(i) / PATCH_LENGTH };

            float nz = static_cast<float>(z - mLength / 2);
            float nx = static_cast<float>(x - mLength / 2);

            data.position.emplace_back(nx, mHeight[z][x], nz);
            data.texCoord1.emplace_back(uv0);
            data.texCoord2.emplace_back(uv1);
        }
    }
}

bool TerrainCollider::LoadFromFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    
    if (!file) {
        return false;
    }

    int patchCount = 0;
    file.read(reinterpret_cast<char*>(&patchCount), sizeof(patchCount));
    mPatches.resize(static_cast<size_t>(patchCount));

    for (int i = 0; i < patchCount; ++i) {
        TessellatedPatchHeader header;
        file.read(reinterpret_cast<char*>(&header.gridWidth), sizeof(header.gridWidth));
        file.read(reinterpret_cast<char*>(&header.gridHeight), sizeof(header.gridHeight));
        file.read(reinterpret_cast<char*>(&header.gridSpacing), sizeof(header.gridSpacing));
        file.read(reinterpret_cast<char*>(&header.minX), sizeof(header.minX));
        file.read(reinterpret_cast<char*>(&header.minZ), sizeof(header.minZ));
        mPatches[static_cast<size_t>(i)].header = header;
    }

    for (int i = 0; i < patchCount; ++i) {
        int count = mPatches[static_cast<size_t>(i)].header.gridWidth * mPatches[static_cast<size_t>(i)].header.gridHeight;
        mPatches[static_cast<size_t>(i)].vertices.resize(static_cast<size_t>(count));
        file.read(reinterpret_cast<char*>(mPatches[static_cast<size_t>(i)].vertices.data()), count * sizeof(SimpleMath::Vector3));
    }
    return true;
}

float TerrainCollider::GetHeight(float x, float z) const {
    for (const auto& patch : mPatches) {
        float patchMinX = patch.header.minX;
        float patchMaxX = patchMinX + patch.header.gridSpacing * (patch.header.gridWidth - 1);
        float patchMinZ = patch.header.minZ;
        float patchMaxZ = patchMinZ + patch.header.gridSpacing * (patch.header.gridHeight - 1);

        if (x >= patchMinX && x <= patchMaxX && z >= patchMinZ && z <= patchMaxZ) {
            float localX = x - patchMinX;
            float localZ = z - patchMinZ;

            float col = localX / patch.header.gridSpacing;
            float row = localZ / patch.header.gridSpacing;
            
            int j = static_cast<int>(std::floor(col));
            int i = static_cast<int>(std::floor(row));
            
            if (i < 0) i = 0;
            if (j < 0) j = 0;
            if (i >= patch.header.gridHeight - 1) i = patch.header.gridHeight - 2;
            if (j >= patch.header.gridWidth - 1) j = patch.header.gridWidth - 2;

            float t = col - static_cast<float>(j);
            float u = row - static_cast<float>(i);

            const SimpleMath::Vector3& v00 = patch.vertices[static_cast<size_t>(i) * patch.header.gridWidth + j];
            const SimpleMath::Vector3& v10 = patch.vertices[static_cast<size_t>(i) * patch.header.gridWidth + (j + 1)];
            const SimpleMath::Vector3& v01 = patch.vertices[static_cast<size_t>(i + 1) * patch.header.gridWidth + j];
            const SimpleMath::Vector3& v11 = patch.vertices[static_cast<size_t>(i + 1) * patch.header.gridWidth + (j + 1)];

            float y0 = v00.y * (1.0f - t) + v10.y * t;
            float y1 = v01.y * (1.0f - t) + v11.y * t;
            return y0 * (1.0f - u) + y1 * u;
        }
    }
    return 0.0f;
}