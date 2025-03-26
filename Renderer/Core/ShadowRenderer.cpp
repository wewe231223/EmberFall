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
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mShadowDSVHeap)));

	mShadowMap = Texture(device, DXGI_FORMAT_D24_UNORM_S8_UINT, SHADOWMAPSIZE<UINT64>, SHADOWMAPSIZE<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	device->CreateDepthStencilView(mShadowMap.GetResource().Get(), nullptr, mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart());

	mShadowCameraBuffer = DefaultBuffer(device, sizeof(CameraConstants), 1, true);
}

void ShadowRenderer::SetShadowDSV(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mShadowMap.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	auto dsv = mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(0, nullptr, FALSE, &dsv);
	commandList->ClearDepthStencilView(mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void ShadowRenderer::Update(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator worldCameraBuffer) {
    SimpleMath::Vector3 cameraPos(-13.0f, 450.0f, -300.0f);


    float pitch = 55.392f * DirectX::XM_PI / 180.0f; 
    float yaw = 0.0f;
    float roll = 0.0f;


    SimpleMath::Matrix rotation = SimpleMath::Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);


    SimpleMath::Vector3 forward = SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3(0, 0, 1), rotation);
    forward.Normalize();


    SimpleMath::Matrix view = DirectX::XMMatrixLookToLH(cameraPos, forward, DirectX::SimpleMath::Vector3::Up);
    SimpleMath::Matrix proj = DirectX::XMMatrixOrthographicLH(1000.f,1000.f, 0.3f, 1000.f);



    mShadowCamera.view = view.Transpose();
    mShadowCamera.proj = proj.Transpose();
    mShadowCamera.viewProj = mShadowCamera.proj * mShadowCamera.view;


	std::memcpy(*mShadowCameraBuffer.CPUBegin(), &mShadowCamera, sizeof(CameraConstants));
    mShadowCameraBuffer.Upload(commandList);
}


void ShadowRenderer::TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mShadowMap.Transition(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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


