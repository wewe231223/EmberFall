#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"
#include "../Utility/Defines.h"

#ifdef max 
#undef max
#endif // max

int TerrainLoader::GetLength() const {
    return mLength;
}

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
		meshData.vertexAttribute.set(1);
        meshData.vertexAttribute.set(2);
        meshData.vertexAttribute.set(3);

    }
    else {
        // 일반 처리 (변경 없음)
    }

    return meshData;
}



SimpleMath::Vector3 TerrainLoader::CalculateNormal(int z, int x) const {
    const int clampZ = std::clamp(z, 1, mLength - 2);
    const int clampX = std::clamp(x, 1, mLength - 2);

    float hl = mHeight[clampZ][clampX - 1]; // left
    float hr = mHeight[clampZ][clampX + 1]; // right
    float hd = mHeight[clampZ + 1][clampX]; // down
    float hu = mHeight[clampZ - 1][clampX]; // up

    DirectX::SimpleMath::Vector3 normal{ hl - hr, 2.f, hd - hu };
    normal.Normalize(); 

    return normal;
}

void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) {

    float stepSize = static_cast<float>(zStart - zEnd) / PATCH_LENGTH;
    
    for (int i = 0; i <= PATCH_LENGTH; ++i) {
        for (int j = 0; j <= PATCH_LENGTH; ++j) {
            int z = std::clamp(static_cast<int>(zStart - i * stepSize), 0, mLength - 1);
            int x = std::clamp(static_cast<int>(xStart + j * stepSize), 0, mLength - 1);

            const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1) };
            const DirectX::XMFLOAT2 uv1{ TILE_SCALE * static_cast<float>(j) / PATCH_LENGTH, TILE_SCALE * static_cast<float>(i) / PATCH_LENGTH};

            float nz = static_cast<float>(z - mLength / 2);
            float nx = static_cast<float>(x - mLength / 2);

            data.position.emplace_back(nx, mHeight[z][x], nz);
			data.normal.emplace_back(CalculateNormal(z, x));
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

	file.read(reinterpret_cast<char*>(&mHeader), sizeof(TerrainHeader));


    mGlobalVertices.resize(mHeader.globalWidth * mHeader.globalHeight);
    file.read(reinterpret_cast<char*>(mGlobalVertices.data()), mGlobalVertices.size() * sizeof(SimpleMath::Vector3));
    return true;
}

TerrainHeader& TerrainCollider::GetHeader() {
    return mHeader; 
}

std::vector<SimpleMath::Vector3>& TerrainCollider::GetData() {
    return mGlobalVertices;
}

float TerrainCollider::GetHeight(float x, float z) const {
    float localX = x - mHeader.minX;
    float localZ = z - mHeader.minZ;

    float fcol = localX / mHeader.gridSpacing;
    float frow = localZ / mHeader.gridSpacing;

    int col = static_cast<int>(fcol);
    int row = static_cast<int>(frow);

	col = std::clamp(col, 0, mHeader.globalWidth - 2);
	row = std::clamp(row, 0, mHeader.globalHeight - 2);

    float t = fcol - col;
    float u = frow - row;

    const SimpleMath::Vector3& v00 = mGlobalVertices[row * mHeader.globalWidth + col];
    const SimpleMath::Vector3& v10 = mGlobalVertices[row * mHeader.globalWidth + col + 1];
    const SimpleMath::Vector3& v01 = mGlobalVertices[(row + 1) * mHeader.globalWidth + col];
    const SimpleMath::Vector3& v11 = mGlobalVertices[(row + 1) * mHeader.globalWidth + col + 1];

    float y0 = v00.y * (1.0f - t) + v10.y * t;
    float y1 = v01.y * (1.0f - t) + v11.y * t;

    return y0 * (1.0f - u) + y1 * u;
}