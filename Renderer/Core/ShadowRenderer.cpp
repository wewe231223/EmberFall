#include "pch.h"
#include "ShadowRenderer.h"
#include "../Utility/Exceptions.h" 
#include "../Game/System/Input.h"
#include "../Game/Scene/Camera.h"

#ifdef max 
#undef max
#endif

#ifdef min
#undef min
#endif

ShadowRenderer::ShadowRenderer(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mShadowRTVHeap)));

	mShadowMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, SHADOWMAPSIZE<UINT64>, SHADOWMAPSIZE<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	device->CreateRenderTargetView(mShadowMap.GetResource().Get(), nullptr, mShadowRTVHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mShadowDSVHeap)));

	mDepthStencilMap = Texture(device, DXGI_FORMAT_D24_UNORM_S8_UINT, SHADOWMAPSIZE<UINT64>, SHADOWMAPSIZE<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);



	device->CreateDepthStencilView(mDepthStencilMap.GetResource().Get(), nullptr, mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart());




	mShadowCameraBuffer = DefaultBuffer(device, sizeof(CameraConstants), 1, true);
}

void ShadowRenderer::SetShadowDSV(ComPtr<ID3D12GraphicsCommandList> commandList) {
	auto rtv = mShadowRTVHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsv = mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->ClearDepthStencilView(mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->ClearRenderTargetView(mShadowRTVHeap->GetCPUDescriptorHandleForHeapStart(), DirectX::Colors::Black, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

}

void ShadowRenderer::Update(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator worldCameraBuffer) {
	// transposed 
	CameraConstants worldCamera{};
	std::memcpy(&worldCamera, *worldCameraBuffer, sizeof(CameraConstants));

	CameraParameter cameraParam{};
	
	
	
	SimpleMath::Matrix invView = worldCamera.view.Transpose().Invert();
	SimpleMath::Matrix invProj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(cameraParam.fov, cameraParam.aspect, cameraParam.nearZ, FRUSTUMLENGTH).Invert();

	SimpleMath::Matrix invViewProj = invProj * invView;
	std::array<SimpleMath::Vector3, 8> frustumPosition = ComputeFrustumCorners(invViewProj);

	SimpleMath::Vector3 centerFrustum = std::accumulate(frustumPosition.begin(), frustumPosition.end(), SimpleMath::Vector3(0.0f, 0.0f, 0.0f)) / 8.0f;

	SimpleMath::Vector3 directionNormalized = LIGHTDIRECTION;
	directionNormalized.Normalize();
	SimpleMath::Vector3 cameraPos(centerFrustum - directionNormalized * (300.0f));
	//SimpleMath::Vector3 cameraPos(centerFrustum - directionNormalized * (FRUSTUMLENGTH - cameraParam.nearZ) );

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
	float nearPadding =40.0f;  // 조명 투영행렬의 근,원평면에 약간의 여유 공간을 추가할때 사용.
	float farPadding = 40.0f;
	float nearPlane = minPoint.z - nearPadding;
	float farPlane = maxPoint.z + farPadding;
	SimpleMath::Matrix proj = SimpleMath::Matrix::CreateOrthographic(projectionSize, projectionSize, nearPlane, farPlane);
	//SimpleMath::Matrix proj = SimpleMath::Matrix::CreateOrthographic(projectionSize, projectionSize, 1.0f, 700.0f);



	mShadowCamera.view = view.Transpose();
	mShadowCamera.proj = proj.Transpose();
	mShadowCamera.viewProj = mShadowCamera.proj * mShadowCamera.view;
	mShadowCamera.cameraPosition = cameraPos;
	mShadowCamera.isShadow = 1;

	std::memcpy(*mShadowCameraBuffer.CPUBegin(), &mShadowCamera, sizeof(CameraConstants));
	mShadowCameraBuffer.Upload(commandList);
}




DefaultBufferGPUIterator ShadowRenderer::GetShadowCameraBuffer() {
	return mShadowCameraBuffer.GPUBegin(); 
}

Texture& ShadowRenderer::GetShadowMap() {
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

void ShadowRenderer::StablizeShadowMatrix(SimpleMath::Matrix& shadowCameraVP) {
	SimpleMath::Matrix texelSizeMatrix = SimpleMath::Matrix::CreateScale(1.0f / SHADOWMAPSIZE<float>, 1.0f / SHADOWMAPSIZE<float>, 1.0f);
	shadowCameraVP = texelSizeMatrix * shadowCameraVP;

	SimpleMath::Matrix roundedMatrix;
	for (int i = 0; i < 3; i++) {
		shadowCameraVP.m[3][i] = std::roundf(shadowCameraVP.m[3][i] * SHADOWMAPSIZE<float>) / SHADOWMAPSIZE<float>;
	}

	shadowCameraVP = SimpleMath::Matrix::CreateScale(SHADOWMAPSIZE<float>, SHADOWMAPSIZE<float>, 1.0f) * shadowCameraVP;
}


