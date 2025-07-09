#pragma once 
#include <chrono> 
using namespace std::chrono_literals;
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Game/GameObject/Transform.h"
#include "../Config/Config.h"
#include "../Game/GameObject/Collider.h"
#include "../Utility/Defines.h"

struct CameraParameter {
	float aspect{Config::WINDOW_WIDTH<float> / Config::WINDOW_HEIGHT<float> };
	float fov{ DirectX::XMConvertToRadians(60.f) };
	float nearZ{ 0.1f };
	float farZ{ 2000.f };
};

class Camera {
public:
	Camera() = default;
	Camera(DefaultBufferCPUIterator bufferLocation);
	~Camera() = default;
public:
	void UpdateBuffer();
	Transform& GetTransform() { return mTransform; }

	bool IsInFrustum(Collider& other) const; 
	bool IsInFrustum(DirectX::BoundingBox& other) const;
public:
	CameraParameter CameraParam{};
private:
	DefaultBufferCPUIterator mCameraBufferCPU{};

	DirectX::BoundingFrustum mViewFrustum{};
	DirectX::BoundingFrustum mWorldFrustum{};

	CameraConstants mCameraConstant{};
	Transform mTransform{};
};

enum class ECameraMode : BYTE {
	Free,
	Follow,
	Lobby, 
};


class CameraMode {
public:
	CameraMode(Camera* camera); 
	virtual ~CameraMode() = default;
public:
	virtual void Enter() PURE;
	virtual void Exit() PURE;
	virtual void Update() PURE;
	virtual void FocusUpdate() PURE;

	virtual ECameraMode GetMode() const PURE; 

	virtual void SetCameraShake(std::chrono::milliseconds duration) PURE;
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
	virtual void FocusUpdate() override;

	virtual ECameraMode GetMode() const override { return ECameraMode::Free; }

	virtual void SetCameraShake(std::chrono::milliseconds duration) override;
};

class TPPCameraMode : public CameraMode {
public:
	TPPCameraMode(Camera* camera, Transform& transform, const DirectX::SimpleMath::Vector3& offset);
	virtual ~TPPCameraMode();
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update() override;
	virtual void FocusUpdate() override;

	virtual ECameraMode GetMode() const;

	virtual void SetCameraShake(std::chrono::milliseconds duration) override;
private:
	Transform& mTargetTransform;
	DirectX::SimpleMath::Vector3 mOffset{ DirectX::SimpleMath::Vector3::Zero };

	std::chrono::milliseconds mShakeDuration{ 0ms };
};
