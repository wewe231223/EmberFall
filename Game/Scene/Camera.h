#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Game/GameObject/Transform.h"
#include "../Config/Config.h"
#include "../Game/GameObject/Collider.h"
#include "../Utility/Defines.h"

struct CameraParameter {
	float aspect{Config::WINDOW_WIDTH<float> / Config::WINDOW_HEIGHT<float> };
	float fov{ DirectX::XMConvertToRadians(60.f) };
	float nearZ{ 0.1f };
	float farZ{ 1000.f };
};

class Camera {
public:
	Camera() = default;
	Camera(DefaultBufferCPUIterator bufferLocation);
	~Camera() = default;
public:
	void UpdateBuffer();
	Transform& GetTransform() { return mTransform; }
	bool FrustumCulling(Collider& other) const; 
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

class TPPCameraMode : public CameraMode {
public:
	TPPCameraMode(Camera* camera, Transform& transform, const DirectX::SimpleMath::Vector3& offset);
	virtual ~TPPCameraMode();
public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update() override;
	virtual ECameraMode GetMode() const;
private:
	Transform& mTargetTransform;
	DirectX::SimpleMath::Vector3 mOffset{ DirectX::SimpleMath::Vector3::Zero };
};

