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

		mControlPoints.insert(mControlPoints.end(), meshData.position.begin(), meshData.position.end());
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

float TerrainLoader::BezierSum(float t, float p0, float p1, float p2, float p3) const {
    float it = 1.0f - t;
    return it * it * it * p0 +
        3 * it * it * t * p1 +
        3 * it * t * t * p2 +
        t * t * t * p3;
}


float TerrainLoader::GetHeightAtL(float x, float z) const {
    //if (mHeight.empty()) return 0.0f;

    //// Height map 중심이 (0,0) 이므로 변환
    //float halfLength = static_cast<float>(mLength) / 2.0f;
    //float fx = x + halfLength;
    //float fz = z + halfLength;

    //// 정수 좌표 계산
    //int x0 = static_cast<int>(std::floor(fx));
    //int x1 = x0 + 1;
    //int z0 = static_cast<int>(std::floor(fz));
    //int z1 = z0 + 1;

    //// 범위 검사: 하나라도 범위를 벗어나면 0 리턴
    //if (x0 < 0 || x1 >= mLength || z0 < 0 || z1 >= mLength)
    //    return 0.0f;

    //// 네 개의 높이값 가져오기
    //float h00 = mHeight[z0][x0];  // 좌상
    //float h10 = mHeight[z0][x1];  // 우상
    //float h01 = mHeight[z1][x0];  // 좌하
    //float h11 = mHeight[z1][x1];  // 우하

    //// 보간 가중치 계산
    //float sx = fx - x0;
    //float sz = fz - z0;

    //// X 방향 보간
    //float h0 = h00 * (1 - sx) + h10 * sx;
    //float h1 = h01 * (1 - sx) + h11 * sx;

    //// Z 방향 보간
    //return h0 * (1 - sz) + h1 * sz;
    if (mHeight.empty()) return 0.0f;

    float halfLength = static_cast<float>(mLength) / 2.0f;
    float fx = x + halfLength;
    float fz = z + halfLength;

    int baseX = static_cast<int>(std::floor(fx)) - 1;
    int baseZ = static_cast<int>(std::floor(fz)) - 1;

    // 범위 검사
    if (baseX < 0 || baseX + 3 >= mLength || baseZ < 0 || baseZ + 3 >= mLength)
        return 0.0f;

    // 4x4 제어점 가져오기
    float controlPoints[4][4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            controlPoints[i][j] = mHeight[baseZ + i][baseX + j];
        }
    }

    // Bézier 보간 함수
    auto Bezier = [](float t, float p0, float p1, float p2, float p3) -> float {
        float u = 1 - t;
        return p0 * (u * u * u) +
            p1 * (3 * u * u * t) +
            p2 * (3 * u * t * t) +
            p3 * (t * t * t);
        };

    // (x, z) 좌표의 상대적 위치
    float u = fx - (baseX + 1);
    float v = fz - (baseZ + 1);

    // Z 방향으로 4개의 Bézier 곡선 평가
    float bezierZ[4];
    for (int i = 0; i < 4; ++i) {
        bezierZ[i] = Bezier(u, controlPoints[i][0], controlPoints[i][1], controlPoints[i][2], controlPoints[i][3]);
    }

    // X 방향으로 최종 Bézier 평가
    return Bezier(v, bezierZ[0], bezierZ[1], bezierZ[2], bezierZ[3]);
}



float TerrainLoader::GetHeightAt(float x, float z) const {
    int half = mLength / 2;

    x += half;
    z += half;

    if (x < 0 || x >= mLength || z < 0 || z >= mLength) {
        return 0.0f; 
    }

    int patchSize = PATCH_LENGTH * PATCH_SCALE;
    int px = static_cast<int>(x) / patchSize * patchSize;
    int pz = static_cast<int>(z) / patchSize * patchSize;

    float u = (x - px) / static_cast<float>(patchSize);
    float v = (z - pz) / static_cast<float>(patchSize);

    DirectX::XMFLOAT3 controlPoints[4][4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int cx = std::clamp(px + j * (patchSize / 3), 0, mLength - 1);
            int cz = std::clamp(pz + i * (patchSize / 3), 0, mLength - 1);
            controlPoints[i][j] = {
                static_cast<float>(cx - half),
                mHeight[cz][cx],
                static_cast<float>(cz - half)
            };
        }
    }

    DirectX::XMFLOAT3 bezierRow[4];
    for (int i = 0; i < 4; ++i) {
        bezierRow[i].y = BezierSum(u, controlPoints[i][0].y, controlPoints[i][1].y, controlPoints[i][2].y, controlPoints[i][3].y);
    }

    float finalHeight = BezierSum(v, bezierRow[0].y, bezierRow[1].y, bezierRow[2].y, bezierRow[3].y);

    return finalHeight;
}

