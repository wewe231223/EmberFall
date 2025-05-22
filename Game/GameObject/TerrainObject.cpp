#include "pch.h"
#include "TerrainObject.h"
#include "../MeshLoader/Loader/TerrainLoader.h"

TerrainSegment::TerrainSegment() {

}

TerrainSegment::TerrainSegment(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const MeshData& data) {
	mMesh = std::make_unique<Mesh>(device, commandList, data);
	DirectX::BoundingBox::CreateFromPoints(mBoundingBox, data.position.size(), data.position.data(), sizeof(DirectX::XMFLOAT3)); 
}

TerrainSegment::~TerrainSegment() {

}

DirectX::BoundingBox& TerrainSegment::GetBB() {
	return mBoundingBox; 
}

Mesh* TerrainSegment::GetMesh() const {
	return mMesh.get();
}

TerrainObject::TerrainObject(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& heightmap) {
	TerrainLoader loader{ heightmap };
	
	auto [width, height] = loader.GetPatchCount();

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			auto data = loader.GetData(i, j);
			mSegments.emplace_back(device, commandList, data);
		}
	}


	mTerrainShader = std::make_shared<TerrainShader>(); 
	mTerrainShader->CreateShader(device); 

	mModelContext.world = SimpleMath::Matrix::Identity;
}

void TerrainObject::SetMaterial(MaterialIndex idx) {
	mModelContext.material = idx;
}

void TerrainObject::Update(Camera& camera, std::shared_ptr<RenderManager> mgr) {
	Mesh* mesh{ nullptr };
	GraphicsShaderBase* shader = mTerrainShader.get(); 

	for (auto& seg : mSegments) {
		mesh = seg.GetMesh();
		if (camera.IsInFrustum(seg.GetBB())) {
			mModelContext.BBCenter = seg.GetBB().Center;
			mModelContext.BBextents = seg.GetBB().Extents;
			mgr->GetMeshRenderManager().AppendPlaneMeshContext(shader, mesh, mModelContext);
		}

		for (auto i = 0; i < Config::SHADOWMAP_COUNT<int>; ++i) {
			if (mgr->GetShadowRenderer().IsInShadowFrustum(i, seg.GetBB())) {
				mgr->GetMeshRenderManager().AppendShadowPlaneMeshContext(shader, mesh, mModelContext, i);
			}
		}

	}
}
