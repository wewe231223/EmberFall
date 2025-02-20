#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Game/GameObject/Transform.h"
#include "../Config/Config.h"

struct CameraParameter {
	float aspect{Config::WINDOW_WIDTH<float> / Config::WINDOW_HEIGHT<float> };
	float fov{ DirectX::XMConvertToRadians(60.f) };
	float nearZ{ 0.1f };
	float farZ{ 1000.f };
};

struct CameraConstant {
	SimpleMath::Matrix view{};
	SimpleMath::Matrix proj{};
	SimpleMath::Matrix viewProj{};
	SimpleMath::Vector3 cameraPos{};
};

class Camera {
public:
	Camera() = default;
	Camera(ComPtr<ID3D12Device> device);
	~Camera() = default;
public:
	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList);
	Transform& GetTransform() { return mTransform; }
public:
	CameraParameter CameraParam{};
private:
	DefaultBuffer mCameraBuffer{};
	CameraConstant mCameraConstant{};
	Transform mTransform{};
};