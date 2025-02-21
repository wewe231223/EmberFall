#include "pch.h"
#include "Camera.h"
#include "../System/Timer.h"
#include "../System/Input.h"
#include "../Utility/NonReplacementSampler.h"
#include "../EditorInterface/Console/Console.h"

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

CameraMode::CameraMode(Camera* camera) : mCamera(camera) {
}

FreeCameraMode::FreeCameraMode(Camera* camera) : CameraMode(camera) {
}

constexpr float FREE_CAMERA_SPEED = 0.5f;
void FreeCameraMode::Enter() {
	mInputCallBackSign = NonReplacementSampler::GetInstance().Sample();

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(mCamera->GetTransform().GetForward() * FREE_CAMERA_SPEED);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::S, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(mCamera->GetTransform().GetForward() * -FREE_CAMERA_SPEED);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, mInputCallBackSign, [this]() {
		auto right = mCamera->GetTransform().GetRight();
		right.y = 0.f;
		mCamera->GetTransform().Translate(right * -FREE_CAMERA_SPEED);
		//mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Right * -0.1f);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, mInputCallBackSign, [this]() {
		auto right = mCamera->GetTransform().GetRight();
		right.y = 0.f;
		mCamera->GetTransform().Translate(right * FREE_CAMERA_SPEED);
		//mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Right * 0.1f);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::Q, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Up * -FREE_CAMERA_SPEED);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::E, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Up * FREE_CAMERA_SPEED);
		});

	mCamera->GetTransform().GetRotation() = DirectX::SimpleMath::Quaternion::Identity;
}

void FreeCameraMode::Exit() {

}

void FreeCameraMode::Update() {

	mCamera->GetTransform().GetRotation() = DirectX::SimpleMath::Quaternion::Identity;

	Console.Log("x : {} , y : {} ", LogType::Info, Input.GetDeltaMouseX(), Input.GetDeltaMouseY());


	static float yaw = 0.f;
	static float pitch = 0.f;

	yaw += Input.GetDeltaMouseX() * Time.GetDeltaTime<float>() * 0.7f;
	pitch += Input.GetDeltaMouseY() * Time.GetDeltaTime<float>() * 0.7f;

	if (pitch > DirectX::XMConvertToRadians(80.f)) {
		pitch = DirectX::XMConvertToRadians(80.f);
	}
	if (pitch < DirectX::XMConvertToRadians(-80.f)) {
		pitch = DirectX::XMConvertToRadians(-80.f);
	}

	mCamera->GetTransform().Rotate(pitch, yaw, 0.f);
}


