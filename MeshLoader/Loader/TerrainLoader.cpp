#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"
#include "../Utility/Defines.h"

#ifdef max 
#undef max
#endif

#ifdef min
#undef min
#endif

TerrainLoader::TerrainLoader(const std::filesystem::path& path) {
    Load(path);
}

void TerrainLoader::Load(const std::filesystem::path& path) {
    CrashExp(path.extension() == ".raw", "Height map should be .raw File");

    size_t size = std::filesystem::file_size(path);
    mLength = static_cast<int>(std::sqrt(size));
    CrashExp(mLength * mLength == static_cast<int>(size), "Height map must be square of BYTE");

    mHeight.assign(mLength, std::vector<float>(mLength));
    std::ifstream file{ path, std::ios::binary };
    std::vector<BYTE> rowData(mLength);
    for (int z = 0; z < mLength; ++z) {
        file.read(reinterpret_cast<char*>(rowData.data()), mLength);
        for (int x = 0; x < mLength; ++x) {
            mHeight[z][x] = static_cast<float>(rowData[x]);
        }
    }

    mMeshData = TerrainLoader::GetData();

    mCPPositions.clear();
    const int patchSize = PATCH_LENGTH * PATCH_SCALE;
    const int numPatches = mLength / patchSize;
    const int numCPsPerRow = numPatches * PATCH_LENGTH + 1;
    mCPPositions.reserve(numCPsPerRow * numCPsPerRow);

    for (int pr = 0; pr <= numPatches * PATCH_LENGTH; ++pr) {
        for (int pc = 0; pc <= numPatches * PATCH_LENGTH; ++pc) {
            int x = pc * TILE_SCALE;
            int z = mLength - 1 - pr * TILE_SCALE;

            float height = 0.0f;
            if (z >= 0 && z < mLength && x >= 0 && x < mLength)
                height = mHeight[z][x];

            mCPPositions.emplace_back(
                static_cast<float>(x),
                height,
                static_cast<float>(z)
            );
        }
    }

    TerrainLoader::SmoothMeshData(2);
}

MeshData TerrainLoader::GetData() const {
	if (mMeshData.position.size() > 0) {
		return mMeshData;
	}


    MeshData meshData;
    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int numPatches = mLength / patchSize;

    for (int pr = 0; pr < numPatches; ++pr) {
        for (int pc = 0; pc < numPatches; ++pc) {
            int zEnd = mLength - (pr + 1) * patchSize;
            int zStart = zEnd + patchSize;
            int xStart = pc * patchSize;
            int xEnd = xStart + patchSize;
            CreatePatch(meshData, zStart, zEnd, xStart, xEnd);
        }
    }

    meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
    meshData.indexed = false;
    meshData.unitCount = static_cast<UINT>(meshData.position.size());

    meshData.vertexAttribute.set(0);
    meshData.vertexAttribute.set(2);
    meshData.vertexAttribute.set(3);

    return meshData;
}

MeshData TerrainLoader::GetData(int patchRow, int patchCol, int mergeCount) const {
    MeshData mergedData;

    const int patchStride = PATCH_LENGTH + 1;
    const int numPatches = mLength / (PATCH_LENGTH * PATCH_SCALE);

    int mergeSize = static_cast<int>(std::sqrt(mergeCount));
    CrashExp(mergeSize * mergeSize == mergeCount, "mergeCount must be perfect square");
    CrashExp(patchRow >= 0 && patchRow < numPatches, "Invalid patchRow");
    CrashExp(patchCol >= 0 && patchCol < numPatches, "Invalid patchCol");

    const int verticesPerPatch = patchStride * patchStride;

    for (int pr = 0; pr < mergeSize; ++pr) {
        for (int pc = 0; pc < mergeSize; ++pc) {
            int currentPatchRow = patchRow + pr;
            int currentPatchCol = patchCol + pc;

            // 경계처리 (넘어가는 경우 → 포함 X)
            if (currentPatchRow >= numPatches || currentPatchCol >= numPatches)
                continue;

            int patchIndex = currentPatchRow * numPatches + currentPatchCol;
            int vertexOffset = patchIndex * verticesPerPatch;

            // 그대로 패치 하나 통째로 append → CP 25개 그대로 유지
            for (int i = 0; i < verticesPerPatch; ++i) {
                mergedData.position.push_back(mMeshData.position[vertexOffset + i]);
                mergedData.texCoord1.push_back(mMeshData.texCoord1[vertexOffset + i]);
                mergedData.texCoord2.push_back(mMeshData.texCoord2[vertexOffset + i]);
            }
        }
    }

    mergedData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
    mergedData.indexed = false;
    mergedData.unitCount = static_cast<UINT>(mergedData.position.size());

    mergedData.vertexAttribute.set(0);
    mergedData.vertexAttribute.set(2);
    mergedData.vertexAttribute.set(3);

    return mergedData;
}


const std::pair<int, int> TerrainLoader::GetPatchCount(int mergeCount) const {
    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int numPatches = mLength / patchSize;

    int mergeSize = static_cast<int>(std::sqrt(mergeCount));
    CrashExp(mergeSize * mergeSize == mergeCount, "mergeCount must be perfect square");

    // 올림 division
    int mergedPatchCount = (numPatches + mergeSize - 1) / mergeSize;

    return { mergedPatchCount, mergedPatchCount };
}
std::vector<SimpleMath::Vector3>& TerrainLoader::GetControlPoints() {
    return mCPPositions;
}

SimpleMath::Vector3 TerrainLoader::CalculateNormal(int z, int x) const {
    const int clampZ = std::clamp(z, 1, mLength - 2);
    const int clampX = std::clamp(x, 1, mLength - 2);

    float hl = mHeight[clampZ][clampX - 1];
    float hr = mHeight[clampZ][clampX + 1];
    float hd = mHeight[clampZ + 1][clampX];
    float hu = mHeight[clampZ - 1][clampX];

    DirectX::SimpleMath::Vector3 normal{ hl - hr, 1.f, hd - hu };
    normal.Normalize();

    return normal;
}

void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) const {
    float stepSize = static_cast<float>(zStart - zEnd) / PATCH_LENGTH;

    int patchStride = PATCH_LENGTH + 1;
    int patchVertexStart = static_cast<int>(data.position.size());

    for (int i = 0; i <= PATCH_LENGTH; ++i) {
        for (int j = 0; j <= PATCH_LENGTH; ++j) {
            int z = std::clamp(static_cast<int>(zStart - i * stepSize), 0, mLength - 1);
            int x = std::clamp(static_cast<int>(xStart + j * stepSize), 0, mLength - 1);

            const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1) };
            const DirectX::XMFLOAT2 uv1{ TILE_SCALE * static_cast<float>(j) / PATCH_LENGTH, TILE_SCALE * static_cast<float>(i) / PATCH_LENGTH };

            float nz = static_cast<float>(z - mLength / 2);
            float nx = static_cast<float>(x - mLength / 2);

            data.position.emplace_back(nx, mHeight[z][x], nz);
            data.texCoord1.emplace_back(uv0);
            data.texCoord2.emplace_back(uv1);
        }
    }
}

void TerrainLoader::SmoothMeshData(int iterations) {
    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int numPatches = mLength / patchSize;

    int patchStride = PATCH_LENGTH + 1;
    int verticesPerPatch = patchStride * patchStride;

    int numCPsPerRow = numPatches * PATCH_LENGTH + 1;

    std::vector<std::vector<float>> heightGrid(numCPsPerRow, std::vector<float>(numCPsPerRow));

    for (int pr = 0; pr < numPatches; ++pr) {
        for (int pc = 0; pc < numPatches; ++pc) {
            int patchIndex = pr * numPatches + pc;
            int vertexOffset = patchIndex * verticesPerPatch;

            for (int i = 0; i <= PATCH_LENGTH; ++i) {
                for (int j = 0; j <= PATCH_LENGTH; ++j) {
                    int globalRow = pr * PATCH_LENGTH + i;
                    int globalCol = pc * PATCH_LENGTH + j;

                    const auto& pos = mMeshData.position[vertexOffset + i * patchStride + j];
                    heightGrid[globalRow][globalCol] = pos.y;
                }
            }
        }
    }

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<std::vector<float>> current = heightGrid;

        for (int r = 0; r < numCPsPerRow; ++r) {
            for (int c = 0; c < numCPsPerRow; ++c) {
                float sum = 0.0f;
                int count = 0;

                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = r + dr;
                        int nc = c + dc;

                        if (nr >= 0 && nr < numCPsPerRow && nc >= 0 && nc < numCPsPerRow) {
                            sum += current[nr][nc];
                            count++;
                        }
                    }
                }

                heightGrid[r][c] = sum / static_cast<float>(count);
            }
        }
    }

    for (int pr = 0; pr < numPatches; ++pr) {
        for (int pc = 0; pc < numPatches; ++pc) {
            int patchIndex = pr * numPatches + pc;
            int vertexOffset = patchIndex * verticesPerPatch;

            for (int i = 0; i <= PATCH_LENGTH; ++i) {
                for (int j = 0; j <= PATCH_LENGTH; ++j) {
                    int globalRow = pr * PATCH_LENGTH + i;
                    int globalCol = pc * PATCH_LENGTH + j;

                    auto& pos = mMeshData.position[vertexOffset + i * patchStride + j];
                    pos.y = heightGrid[globalRow][globalCol];
                }
            }
        }
    }
}


namespace Client {
    bool TerrainCollider::LoadFromFile(const std::filesystem::path& filePath) {
        std::ifstream file(filePath, std::ios::binary);

        if (!file) {
			mHeader.globalWidth = 0;
			mHeader.globalHeight = 0;

            return true; 
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
        if (mHeader.minX == 0 and mHeader.minZ == 0) {
            return 0.f; 
        }


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
}