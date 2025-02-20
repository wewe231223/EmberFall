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

enum class ECameraMode : BYTE {
	Free,
	Follow,
};

class CameraMode {
public:
	CameraMode(Camera* camera); 
	virtual ~CameraMode() = default;
public:
	virtual void Enter() PURE;
	virtual void Exit() PURE;
	virtual void Update() PURE;

	virtual ECameraMode GetMode() const PURE; 
protected:
	Camera* mCamera{ nullptr };
	int mInputCallBackSign{ -1 };
};

class FreeCameraMode : public CameraMode {
public:
	FreeCameraMode(Camera* camera);
	virtual ~FreeCameraMode() = default;
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update() override;
	
	virtual ECameraMode GetMode() const override { return ECameraMode::Free; }
};