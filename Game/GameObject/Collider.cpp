#include "pch.h"
#include "Collider.h"
#include "../GameObject/GameObject.h"

Collider::Collider(std::vector<DirectX::XMFLOAT3>& positions) {
	DirectX::BoundingOrientedBox::CreateFromPoints(mOrigin, positions.size(), positions.data(), sizeof(DirectX::XMFLOAT3));
	mActive = true;
}

bool Collider::GetActiveState() const {
	return mActive;
}

void Collider::UpdateBox(SimpleMath::Matrix& world) {
	mOrigin.Transform(mWorld, world);
}

SimpleMath::Vector3 Collider::GetExtents() const {
	return mOrigin.Extents;
}

bool Collider::CheckCollision(Collider& other) {
	return mWorld.Intersects(other.mWorld);
}
