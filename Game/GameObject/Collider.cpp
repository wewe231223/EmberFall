#include "pch.h"
#include "Collider.h"
#include "../GameObject/GameObject.h"

Collider::Collider(std::vector<DirectX::XMFLOAT3>& positions) {
	DirectX::BoundingBox box{};
	DirectX::BoundingBox::CreateFromPoints(box, positions.size(), positions.data(), sizeof(DirectX::XMFLOAT3));
	DirectX::BoundingOrientedBox::CreateFromBoundingBox(mOrigin, box);
	mActive = true;
}

Collider::Collider(DirectX::BoundingBox box) {
	DirectX::BoundingOrientedBox::CreateFromBoundingBox(mOrigin, box);
	mActive = true;
}

bool Collider::GetActiveState() const {
	return mActive;
}

void Collider::UpdateBox(SimpleMath::Matrix& world) {
	mOrigin.Transform(mWorld, world);
}

SimpleMath::Vector3 Collider::GetCenter() const {
	return mWorld.Center;
}

SimpleMath::Vector3 Collider::GetExtents() const {
	return mWorld.Extents;
}

bool Collider::CheckCollision(Collider& other) {
	return mWorld.Intersects(other.mWorld);
}
