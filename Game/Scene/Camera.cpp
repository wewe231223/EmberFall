#include "pch.h"
#include "Camera.h"

Camera::Camera(ComPtr<ID3D12Device> device) {
	mCameraBuffer = DefaultBuffer(device.Get(), sizeof(CameraConstant), 1, true);

	mCameraConstant.proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(CameraParam.fov, CameraParam.aspect, CameraParam.nearZ, CameraParam.farZ).Transpose();
}

void Camera::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) {

	mCameraConstant.view = SimpleMath::Matrix::CreateLookAt(mTransform.GetPosition(), mTransform.GetPosition() + mTransform.GetForward(),SimpleMath::Vector3::Up).Transpose();
	mCameraConstant.viewProj = mCameraConstant.proj * mCameraConstant.view;
	mCameraConstant.cameraPos = mTransform.GetPosition();

	::memcpy(mCameraBuffer.Data(), &mCameraConstant, sizeof(CameraConstant));
	mCameraBuffer.Upload(commandList);

	commandList->SetGraphicsRootConstantBufferView(0, *mCameraBuffer.GPUBegin());
}
