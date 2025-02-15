#include "pch.h"
#include "Collider.h"
#include "../GameObject/GameObject.h"

Collider::Collider(GameObject* owner, DirectX::BoundingOrientedBox box, std::function<void(GameObject*, GameObject*)> onCollision) : mOwner(owner), mOriginBox(box), mOnCollision(onCollision) {
}

void Collider::Update(Transform& transform) {
	mOriginBox.Transform(mBox, transform.GetWorldMatrix());
}

void Collider::Test(Collider& other) {
	if (mBox.Intersects(other.mBox)) {
		if (mOnCollision) {
			std::invoke(mOnCollision, mOwner, other.mOwner);
		}

		for (auto& box : mCompositeBox) {
			box.Test(other);
		}

		for (auto& box : other.mCompositeBox) {
			Test(box);
		}
	}
}

void FrustumCollider::InitializeViewFrustum(SimpleMath::Matrix& proj) {
	DirectX::BoundingFrustum::CreateFromMatrix(mViewFrustum, proj);
}

void FrustumCollider::Update(SimpleMath::Matrix& view) {
	mViewFrustum.Transform(mWorldFrustum, view.Invert());
}

bool FrustumCollider::Intersect(Collider& other) {
    return mWorldFrustum.Intersects(other.mBox);
}
