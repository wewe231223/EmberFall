#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"
#include "../Utility/Defines.h"

#ifdef max 
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

TerrainLoader::TerrainLoader(const std::filesystem::path& path) {
	Load(path);
}

void TerrainLoader::Load(const std::filesystem::path& path) {
    CrashExp(path.extension() == ".raw", "Height map should be .raw File");

    size_t size = std::filesystem::file_size(path);
    mLength = static_cast<int>(std::sqrt(size));
    CrashExp(mLength * mLength == static_cast<int>(size),
        "Height map must be square of BYTE");

    // 읽기
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
}

MeshData TerrainLoader::GetData() const {
    MeshData meshData;
    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int numPatches = mLength / patchSize;

    // 모든 패치 생성
    for (int pr = 0; pr < numPatches; ++pr) {
        for (int pc = 0; pc < numPatches; ++pc) {
            int zEnd = mLength - (pr + 1) * patchSize;
            int zStart = zEnd + patchSize;
            int xStart = pc * patchSize;
            int xEnd = xStart + patchSize;
            CreatePatch(meshData, zStart, zEnd, xStart, xEnd);
        }
    }

    // 공통 설정
    meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
    meshData.indexed = false;
    meshData.unitCount = static_cast<UINT>(meshData.position.size());

    meshData.vertexAttribute.set(0);
    meshData.vertexAttribute.set(2);
    meshData.vertexAttribute.set(3);

    return meshData;
}   

MeshData TerrainLoader::GetData(int patchRow, int patchCol) const {
    MeshData patchData;

    const int patchStride = PATCH_LENGTH + 1;
    const int numPatches = mLength / (PATCH_LENGTH * PATCH_SCALE);

    CrashExp(patchRow >= 0 && patchRow < numPatches, "Invalid patchRow");
    CrashExp(patchCol >= 0 && patchCol < numPatches, "Invalid patchCol");

    // 전체 패치 순서상 인덱스
    const int patchIndex = patchRow * numPatches + patchCol;
    const int verticesPerPatch = patchStride * patchStride;
    const int vertexOffset = patchIndex * verticesPerPatch;

    for (int i = 0; i < verticesPerPatch; ++i) {
        patchData.position.push_back(mMeshData.position[vertexOffset + i]);
        patchData.texCoord1.push_back(mMeshData.texCoord1[vertexOffset + i]);
        patchData.texCoord2.push_back(mMeshData.texCoord2[vertexOffset + i]);
    }

    patchData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
    patchData.indexed = false;
    patchData.unitCount = static_cast<UINT>(patchData.position.size());
    
	patchData.vertexAttribute.set(0);
	patchData.vertexAttribute.set(2);
	patchData.vertexAttribute.set(3);

    return patchData;
}


const std::pair<int, int> TerrainLoader::GetPatchCount() const {
    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int numPatches = mLength / patchSize;

	return { numPatches, numPatches };
}

std::vector<SimpleMath::Vector3>& TerrainLoader::GetControlPoints() {
	return mCPPositions;
}

SimpleMath::Vector3 TerrainLoader::CalculateNormal(int z, int x) const {
    const int clampZ = std::clamp(z, 1, mLength - 2);
    const int clampX = std::clamp(x, 1, mLength - 2);

    float hl = mHeight[clampZ][clampX - 1]; // left
    float hr = mHeight[clampZ][clampX + 1]; // right
    float hd = mHeight[clampZ + 1][clampX]; // down
    float hu = mHeight[clampZ - 1][clampX]; // up

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

bool TerrainCollider::LoadFromFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);

    if (!file) {
        return false;
    }

    auto size = std::filesystem::file_size(filePath);
    int sourceLength = static_cast<int>(std::sqrt(size));  // 1025 expected

    std::vector<BYTE> sourceData{};
    sourceData.resize(size);

    file.read(reinterpret_cast<char*>(sourceData.data()), size);

    // Target resolution: 2048 x 2048
    const int targetLength = 8192;
    mLength = targetLength;
    mPixels = std::make_shared<float[]>(targetLength * targetLength);

    // Perform bilinear upscaling
    for (int i = 0; i < targetLength; ++i) {
        for (int j = 0; j < targetLength; ++j) {
            // Map target pixel to source space
            float u = static_cast<float>(j) / (targetLength - 1);
            float v = static_cast<float>(i) / (targetLength - 1);

            float srcX = u * (sourceLength - 1);
            float srcY = v * (sourceLength - 1);

            int ix = static_cast<int>(srcX);
            int iy = static_cast<int>(srcY);

            float fx = srcX - ix;
            float fy = srcY - iy;

            // Clamp to source bounds
            ix = std::clamp(ix, 0, sourceLength - 2);
            iy = std::clamp(iy, 0, sourceLength - 2);

            float h00 = static_cast<float>(sourceData[ix + iy * sourceLength]);
            float h10 = static_cast<float>(sourceData[(ix + 1) + iy * sourceLength]);
            float h01 = static_cast<float>(sourceData[ix + (iy + 1) * sourceLength]);
            float h11 = static_cast<float>(sourceData[(ix + 1) + (iy + 1) * sourceLength]);

            // Bilinear interpolation
            float h0 = h00 * (1.0f - fx) + h10 * fx;
            float h1 = h01 * (1.0f - fx) + h11 * fx;
            float finalHeight = h0 * (1.0f - fy) + h1 * fy;

            // Store flipped vertically (same as your original code)
            mPixels[j + ((targetLength - i - 1) * targetLength)] = finalHeight;
        }
    }

    return true;
}

std::shared_ptr<float[]>& TerrainCollider::GetData() {
    return mPixels;
}

float TerrainCollider::GetHeight(float x, float z) const {

    const float terrainWorldSizeX = 1025.f;  
    const float terrainWorldSizeZ = 1025.f;  

    // Transform world x,z → u,v (0~1)
    float u = (x + terrainWorldSizeX * 0.5f) / terrainWorldSizeX;
    float v = (z + terrainWorldSizeZ * 0.5f) / terrainWorldSizeZ;

    // Clamp to 0~1 range
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    // Map u,v → texture space
    float texX = u * (mLength - 1);
    float texZ = v * (mLength - 1);

    int ix = static_cast<int>(texX);
    int iz = static_cast<int>(texZ);

    float fx = texX - ix;
    float fz = texZ - iz;

    float* pixels = mPixels.get();

    float h00 = pixels[ix + iz * mLength];
    float h10 = pixels[(ix + 1) + iz * mLength];
    float h01 = pixels[ix + (iz + 1) * mLength];
    float h11 = pixels[(ix + 1) + (iz + 1) * mLength];

    float h0 = Lerp(h00, h10, fx);
    float h1 = Lerp(h01, h11, fx);
    float finalHeight = Lerp(h0, h1, fz);

    return finalHeight;
}