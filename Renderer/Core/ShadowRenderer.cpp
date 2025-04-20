#include "pch.h"
#include "ShadowRenderer.h"
#include "../Utility/Exceptions.h" 
#include "../Game/System/Input.h"


#ifdef max 
#undef max
#endif

#ifdef min
#undef min
#endif

ShadowRenderer::ShadowRenderer(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = Config::SHADOWMAP_COUNT<int>;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mShadowRTVHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapHandle{ mShadowRTVHeap->GetCPUDescriptorHandleForHeapStart() };

	for (Texture& shadowMap : mShadowMap) {
		shadowMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, SHADOWMAPSIZE<UINT64>, SHADOWMAPSIZE<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

	}
	
	for (Texture& shadowMap : mShadowMap) {
		device->CreateRenderTargetView(shadowMap.GetResource().Get(), nullptr, shadowMapHandle);
		shadowMapHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	}


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mShadowDSVHeap)));

	mDepthStencilMap = Texture(device, DXGI_FORMAT_D24_UNORM_S8_UINT, SHADOWMAPSIZE<UINT64>, SHADOWMAPSIZE<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);



	device->CreateDepthStencilView(mDepthStencilMap.GetResource().Get(), nullptr, mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart());



	for (DefaultBuffer& shadowCameraBuffer : mShadowCameraBuffer) {
		shadowCameraBuffer = DefaultBuffer(device, sizeof(CameraConstants), 1, true);
	}
	
}

void ShadowRenderer::SetShadowDSVRTV(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, int index) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{ mShadowRTVHeap->GetCPUDescriptorHandleForHeapStart() };
	auto dsv = mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart();
	rtv.Offset(index, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	commandList->ClearDepthStencilView(mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->ClearRenderTargetView(rtv, DirectX::Colors::Black, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);



}

void ShadowRenderer::Update(DefaultBufferCPUIterator worldCameraBuffer) {
	// transposed 
	CameraConstants worldCamera{};
	std::memcpy(&worldCamera, *worldCameraBuffer, sizeof(CameraConstants));

	CameraParameter cameraParam{};
	SimpleMath::Matrix invView = worldCamera.view.Transpose().Invert();

	mWorldBox.clear();

	mLightMatrix[0] = ComputeLightViewMatrix(cameraParam, invView, cameraParam.nearZ, SHADOWMAPOFFSET[0]);
	mLightMatrix[1] = ComputeLightViewMatrix(cameraParam, invView, SHADOWMAPOFFSET[0], SHADOWMAPOFFSET[1]);
	//mLightMatrix[2] = ComputeLightViewMatrix(cameraParam, invView, SHADOWMAPOFFSET[1], SHADOWMAPOFFSET[2]);

	for (SimpleMath::Matrix& lightMatrix : mLightMatrix) {
		StablizeShadowMatrix(lightMatrix);
	}


	mShadowCamera.view = worldCamera.view;
	mShadowCamera.viewProj = mLightMatrix[0];
	mShadowCamera.middleViewProj = mLightMatrix[1];
	//mShadowCamera.farViewProj = mLightMatrix[2];

	mShadowCamera.cameraPosition = worldCamera.cameraPosition;
	mShadowCamera.lengthOffset.x = SHADOWMAPOFFSET[0];
	mShadowCamera.lengthOffset.y = SHADOWMAPOFFSET[1];
	//mShadowCamera.lengthOffset.z = SHADOWMAPOFFSET[2];

	mShadowCamera.isShadow = 1;

	std::memcpy(*mShadowCameraBuffer[0].CPUBegin(), &mShadowCamera, sizeof(CameraConstants));

	mShadowCamera.viewProj = mLightMatrix[1];
	std::memcpy(*mShadowCameraBuffer[1].CPUBegin(), &mShadowCamera, sizeof(CameraConstants));
	
	//mShadowCamera.viewProj = mLightMatrix[2];
	//std::memcpy(*mShadowCameraBuffer[2].CPUBegin(), &mShadowCamera, sizeof(CameraConstants));


}

void ShadowRenderer::Upload(ComPtr<ID3D12GraphicsCommandList> commandList) {
	for (DefaultBuffer& shadowCameraBuffer : mShadowCameraBuffer) {
		shadowCameraBuffer.Upload(commandList);
	}
}



bool ShadowRenderer::ShadowMapCulling(int index, Collider& other) {
	auto& box = other.GetWorldBox();

	return mWorldBox[index].Intersects(box);
}

void ShadowRenderer::TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	
	for (Texture& texture : mShadowMap) {
		texture.Transition(commandList, before, after);
	}
}




DefaultBufferGPUIterator ShadowRenderer::GetShadowCameraBuffer(int index) {
	return mShadowCameraBuffer[index].GPUBegin();
}

Texture& ShadowRenderer::GetShadowMap(int index) {
	return mShadowMap[index];
}

std::array<Texture, Config::SHADOWMAP_COUNT<int>>& ShadowRenderer::GetShadowMapArray() {
	return mShadowMap;
}

std::array<SimpleMath::Vector3, 8> ShadowRenderer::ComputeFrustumCorners(SimpleMath::Matrix worldCameraVPInv) {
	std::array<SimpleMath::Vector3, 8> frustumCorners{};

	SimpleMath::Vector4 ndcCorners[8] = {
		{-1,  1, 0, 1}, { 1,  1, 0, 1}, // Near Plane
		{ 1, -1, 0, 1}, {-1, -1, 0, 1},

		{-1,  1, 1, 1}, { 1,  1, 1, 1}, // Far Plane
		{ 1, -1, 1, 1}, {-1, -1, 1, 1}
	};

	for (int i = 0; i < 8; ++i) {
		SimpleMath::Vector4 corner = SimpleMath::Vector4::Transform(ndcCorners[i], worldCameraVPInv);
		frustumCorners[i] = (SimpleMath::Vector3(corner.x, corner.y, corner.z) / corner.w);
	}

	return frustumCorners;
}

SimpleMath::Matrix ShadowRenderer::ComputeLightViewMatrix(CameraParameter cameraParam, SimpleMath::Matrix invView, float nearZ, float farZ) {


	SimpleMath::Matrix invProj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(cameraParam.fov, cameraParam.aspect, nearZ, farZ).Invert();

	SimpleMath::Matrix invViewProj = invProj * invView;
	std::array<SimpleMath::Vector3, 8> frustumPosition = ComputeFrustumCorners(invViewProj);

	SimpleMath::Vector3 centerFrustum = std::accumulate(frustumPosition.begin(), frustumPosition.end(), SimpleMath::Vector3(0.0f, 0.0f, 0.0f)) / 8.0f;

	SimpleMath::Vector3 directionNormalized = LIGHTDIRECTION;
	directionNormalized.Normalize();
	//SimpleMath::Vector3 cameraPos(centerFrustum - directionNormalized * (150.0f));
	SimpleMath::Vector3 cameraPos(centerFrustum - directionNormalized * (farZ - nearZ) );

	SimpleMath::Matrix view = SimpleMath::Matrix::CreateLookAt(cameraPos, centerFrustum, DirectX::SimpleMath::Vector3::Up);




	for (SimpleMath::Vector3& position : frustumPosition) {
		position = SimpleMath::Vector3::Transform(position, view);
	}

	SimpleMath::Vector3 minPoint = frustumPosition[0];
	SimpleMath::Vector3 maxPoint = frustumPosition[0];



	for (const SimpleMath::Vector3& position : frustumPosition)
	{
		minPoint.x = std::min(minPoint.x, position.x);
		minPoint.y = std::min(minPoint.y, position.y);
		minPoint.z = std::min(minPoint.z, position.z);

		maxPoint.x = std::max(maxPoint.x, position.x);
		maxPoint.y = std::max(maxPoint.y, position.y);
		maxPoint.z = std::max(maxPoint.z, position.z);
	}





	float projectionSize = std::max(maxPoint.x - minPoint.x, maxPoint.y - minPoint.y);
	
	float nearPlane = minPoint.z - NEAROFFSET;
	float farPlane = maxPoint.z + FAROFFSET;

	

	SimpleMath::Matrix proj = SimpleMath::Matrix::CreateOrthographic(projectionSize + PROJECTIONOFFSET, projectionSize + PROJECTIONOFFSET, nearPlane, farPlane);




	SimpleMath::Matrix shadowInverse = proj.Invert() * view.Invert();
	ComputeOrientedBoundingBox(shadowInverse);

	return proj.Transpose() * view.Transpose();
}

void ShadowRenderer::ComputeOrientedBoundingBox(SimpleMath::Matrix cameraProjInv) {
	
	std::array<SimpleMath::Vector3, 8> ndcCorners = {
	SimpleMath::Vector3(-1,  1, 0), // Near Plane
	SimpleMath::Vector3(1,  1, 0),
	SimpleMath::Vector3(1, -1, 0),
	SimpleMath::Vector3(-1, -1, 0),
	SimpleMath::Vector3(-1,  1, 1), // Far Plane
	SimpleMath::Vector3(1,  1, 1),
	SimpleMath::Vector3(1, -1, 1),
	SimpleMath::Vector3(-1, -1, 1)
	};

	DirectX::BoundingOrientedBox worldBox;
	DirectX::BoundingOrientedBox::CreateFromPoints(worldBox, 8, ndcCorners.data(), sizeof(SimpleMath::Vector3));

	worldBox.Transform(worldBox, cameraProjInv);
	mWorldBox.emplace_back(worldBox);

}

void ShadowRenderer::StablizeShadowMatrix(SimpleMath::Matrix& shadowCameraVP) {
	SimpleMath::Matrix texelSizeMatrix = SimpleMath::Matrix::CreateScale(1.0f / SHADOWMAPSIZE<float>, 1.0f / SHADOWMAPSIZE<float>, 1.0f);
	shadowCameraVP = texelSizeMatrix * shadowCameraVP;

	SimpleMath::Matrix roundedMatrix;
	for (int i = 0; i < 3; i++) {
		shadowCameraVP.m[3][i] = std::roundf(shadowCameraVP.m[3][i] * SHADOWMAPSIZE<float>) / SHADOWMAPSIZE<float>;
	}

	shadowCameraVP = SimpleMath::Matrix::CreateScale(SHADOWMAPSIZE<float>, SHADOWMAPSIZE<float>, 1.0f) * shadowCameraVP;
}


