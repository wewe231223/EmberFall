#include "pch.h"
#include "TerrainObject.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#include <algorithm>
#include <execution>

TerrainSegment::TerrainSegment() {

}

TerrainSegment::TerrainSegment(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const MeshData& data, int x, int z) {
	mMesh = std::make_unique<Mesh>(device, commandList, data);
	DirectX::BoundingBox::CreateFromPoints(mBoundingBox, data.position.size(), data.position.data(), sizeof(DirectX::XMFLOAT3));

	mSegmentContext.prevWorld = SimpleMath::Matrix::Identity;
	mSegmentContext.world = SimpleMath::Matrix::Identity;

	mSegmentContext.BBCenter = mBoundingBox.Center;
	mSegmentContext.BBextents = mBoundingBox.Extents;

	mSegmentContext.xPatchIndex = x;
	mSegmentContext.zPatchIndex = z;
}

TerrainSegment::~TerrainSegment() {

}

DirectX::BoundingBox& TerrainSegment::GetBB() {
	return mBoundingBox;
}

Mesh* TerrainSegment::GetMesh() const {
	return mMesh.get();
}

TerrainSegmentContext& TerrainSegment::GetContext() {
	return mSegmentContext;
}

void TerrainSegment::SetMaterial(MaterialIndex idx) {
	mSegmentContext.material = idx;
}

TerrainObject::TerrainObject(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& heightmap) {
	TerrainLoader loader{ heightmap };

	int MERGE_COUNT = 25;

	auto [width, height] = loader.GetPatchCount(MERGE_COUNT);

	struct PatchIndex {
		int i, j;
	};

	std::vector<PatchIndex> indices;
	indices.reserve(width * height);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			indices.push_back({ i, j });
		}
	}

	std::vector<std::tuple<MeshData, int, int>> patchData;
	patchData.resize(indices.size());

	std::transform(std::execution::par, indices.begin(), indices.end(), patchData.begin(),
		[&](const PatchIndex& idx) {
			int mergeSize = static_cast<int>(std::sqrt(MERGE_COUNT));
			int patchRow = idx.i * mergeSize;
			int patchCol = idx.j * mergeSize;

			MeshData data = loader.GetData(patchRow, patchCol, MERGE_COUNT);

			bool withinBounds = std::none_of(data.position.begin(), data.position.end(),
				[](const DirectX::XMFLOAT3& pos) {
					return pos.x <= -300.f || pos.x >= 350.f || pos.z <= -300.f || pos.z >= 300.f;
				});

			if (!withinBounds) {
				return std::make_tuple(MeshData{}, -1, -1);
			}

			return std::make_tuple(std::move(data), idx.i, idx.j);
		});

	for (auto& [data, i, j] : patchData) {
		if (data.position.empty() || i < 0 || j < 0)
			continue;

		mSegments.emplace_back(device, commandList, data, i, j);
	}

	mTerrainShader = std::make_shared<TerrainShader>();
	mTerrainShader->CreateShader(device);

	auto& cpPos = loader.GetControlPoints();
	mCPPositionBuffer = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector3), cpPos.size(), cpPos.data());
}

void TerrainObject::SetMaterial(MaterialIndex idx) {
	for (auto& seg : mSegments) {
		seg.SetMaterial(idx);
	}
}

DefaultBufferGPUIterator TerrainObject::GetCPPositionBuffer() {
	return mCPPositionBuffer.GPUBegin();
}

void TerrainObject::Update(Camera& camera, std::shared_ptr<RenderManager> mgr) {
	Mesh* mesh{ nullptr };
	GraphicsShaderBase* shader = mTerrainShader.get();

	for (auto& seg : mSegments) {
		mesh = seg.GetMesh();
		auto& context = seg.GetContext();

		if (camera.IsInFrustum(seg.GetBB())) {
			mgr->GetMeshRenderManager().AppendTerrainMeshContext(shader, mesh, context);
		}

		for (auto i = 0; i < Config::SHADOWMAP_COUNT<int>; ++i) {
			if (mgr->GetShadowRenderer().IsInShadowFrustum(i, seg.GetBB())) {
				mgr->GetMeshRenderManager().AppendShadowTerrainMeshContext(shader, mesh, context, i);
			}
		}
	}
}
