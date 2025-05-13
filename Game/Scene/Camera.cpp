#include "pch.h"
#include "Camera.h"
#include "../System/Timer.h"
#include "../System/Input.h"
#include "../Utility/NonReplacementSampler.h"
#include "../Renderer/Core/Console.h"

Camera::Camera(DefaultBufferCPUIterator bufferLocation) : mCameraBufferCPU(bufferLocation) {
	mCameraConstant.proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(CameraParam.fov, CameraParam.aspect, CameraParam.nearZ, CameraParam.farZ).Transpose();
	DirectX::BoundingFrustum::CreateFromMatrix(mViewFrustum, mCameraConstant.proj.Transpose());
	mCameraConstant.proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(CameraParam.fov, CameraParam.aspect, CameraParam.farZ, CameraParam.nearZ).Transpose();

}

void Camera::UpdateBuffer() {

	mCameraConstant.view = SimpleMath::Matrix::CreateLookAt(mTransform.GetPosition(), mTransform.GetPosition() + mTransform.GetForward(),SimpleMath::Vector3::Up).Transpose();
	mCameraConstant.viewProj = mCameraConstant.proj * mCameraConstant.view;
	mCameraConstant.cameraPosition = mTransform.GetPosition();

	mViewFrustum.Transform(mWorldFrustum, SimpleMath::Matrix::CreateLookAt(mTransform.GetPosition(), mTransform.GetPosition() + mTransform.GetForward(), SimpleMath::Vector3::Up).Invert());

	::memcpy(*mCameraBufferCPU, &mCameraConstant, sizeof(CameraConstants));
}

bool Camera::FrustumCulling(Collider& other) const {
	auto& box = other.GetWorldBox();

	return mWorldFrustum.Intersects(box);
}

CameraMode::CameraMode(Camera* camera) : mCamera(camera) {
}

FreeCameraMode::FreeCameraMode(Camera* camera) : CameraMode(camera) {
}

constexpr float FREE_CAMERA_SPEED = 25.f;
void FreeCameraMode::Enter() {
	mInputCallBackSign = NonReplacementSampler::GetInstance().Sample();

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(mCamera->GetTransform().GetForward() * FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::S, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(mCamera->GetTransform().GetForward() * -FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, mInputCallBackSign, [this]() {
		auto right = mCamera->GetTransform().GetRight();
		right.y = 0.f;
		mCamera->GetTransform().Translate(right * -FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		//mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Right * -0.1f);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, mInputCallBackSign, [this]() {
		auto right = mCamera->GetTransform().GetRight();
		right.y = 0.f;
		mCamera->GetTransform().Translate(right * FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		//mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Right * 0.1f);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::Q, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Up * -FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::E, mInputCallBackSign, [this]() {
		mCamera->GetTransform().Translate(DirectX::SimpleMath::Vector3::Up * FREE_CAMERA_SPEED * Time.GetSmoothDeltaTime<float>());
		});

	mCamera->GetTransform().GetRotation() = DirectX::SimpleMath::Quaternion::Identity;

}

void FreeCameraMode::Exit() {

}

void FreeCameraMode::Update() {

	mCamera->GetTransform().GetRotation() = DirectX::SimpleMath::Quaternion::Identity;

	static float yaw = 0.f;
	static float pitch = 0.f;

	yaw += Input.GetDeltaMouseX() * Time.GetSmoothDeltaTime<float>() * 0.7f;
	pitch += Input.GetDeltaMouseY() * Time.GetSmoothDeltaTime<float>() * 0.7f;

	if (pitch > DirectX::XMConvertToRadians(80.f)) {
		pitch = DirectX::XMConvertToRadians(80.f);
	}
	if (pitch < DirectX::XMConvertToRadians(-80.f)) {
		pitch = DirectX::XMConvertToRadians(-80.f);
	}

	mCamera->GetTransform().Rotate(pitch, yaw, 0.f);
}

void FreeCameraMode::FocusUpdate() {

}

TPPCameraMode::TPPCameraMode(Camera* camera, Transform& transform, const DirectX::SimpleMath::Vector3& offset) : CameraMode(camera), mOffset(offset), mTargetTransform(transform) {
}

TPPCameraMode::~TPPCameraMode() {
}

void TPPCameraMode::Enter() {
}

void TPPCameraMode::Exit() {
}

void TPPCameraMode::Update() {
	const float YSensitive = 0.2f;

	static float pitch = 0.f;


	pitch -= Input.GetDeltaMouseY() * Time.GetSmoothDeltaTime<float>() * 0.3f;
	pitch = std::clamp(pitch, DirectX::XMConvertToRadians(-35.f), DirectX::XMConvertToRadians(65.f));

	auto forward = mTargetTransform.GetForward();
	auto right = mTargetTransform.GetRight();
	auto up = mTargetTransform.GetUp();

	DirectX::SimpleMath::Matrix pitchRotation = DirectX::SimpleMath::Matrix::CreateFromAxisAngle(right, pitch);

	DirectX::SimpleMath::Vector3 rotatedForward = DirectX::SimpleMath::Vector3::Transform(forward, pitchRotation);
	DirectX::SimpleMath::Vector3 rotatedUp = DirectX::SimpleMath::Vector3::Transform(up, pitchRotation);

	DirectX::SimpleMath::Vector3 camPos =
		mTargetTransform.GetPosition() +
		mOffset.x * right +
		mOffset.y * rotatedUp +
		mOffset.z * rotatedForward;

	mCamera->GetTransform().SetPosition(camPos);


	DirectX::SimpleMath::Vector3 flatDir = mTargetTransform.GetPosition() - camPos;
	flatDir.y = 0;
	flatDir.Normalize();
	float yaw = std::atan2(flatDir.x, flatDir.z);

	mCamera->GetTransform().GetRotation() = DirectX::SimpleMath::Quaternion::Identity;
	//mCamera->GetTransform().Rotate(0.f, yaw, 0.f);


}

void TPPCameraMode::FocusUpdate() {
	auto targetPos = mTargetTransform.GetPosition();
	targetPos.y += 0.9f;
	mCamera->GetTransform().Look(targetPos);
}

ECameraMode TPPCameraMode::GetMode() const {
	return ECameraMode();
}
