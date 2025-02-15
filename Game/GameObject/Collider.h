#pragma once 
#include <functional>
#include "../Utility/DirectXInclude.h"
#include "../Game/GameObject/Transform.h"
class GameObject;

class Collider {
	friend class FrustumCollider;
public:
	Collider() = default;
	Collider(GameObject* owner, DirectX::BoundingOrientedBox box, std::function<void(GameObject*, GameObject*)> onCollision = nullptr);
	~Collider() = default;
public:
	void Update(Transform& transform);
	void Test(Collider& other);
private:
	DirectX::BoundingOrientedBox mOriginBox{};
	DirectX::BoundingOrientedBox mBox{};
	std::function<void(GameObject*,GameObject*)> mOnCollision{};
	std::vector<Collider> mCompositeBox{};

	GameObject* mOwner{ nullptr };
};


class FrustumCollider {
public:
	FrustumCollider() = default;
	~FrustumCollider() = default;
public:
	void InitializeViewFrustum(SimpleMath::Matrix& proj);
	void Update(SimpleMath::Matrix& view); 
	bool Intersect(Collider& other);
private:
	DirectX::BoundingFrustum mViewFrustum{};
	DirectX::BoundingFrustum mWorldFrustum{};
};