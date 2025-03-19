#pragma once 
#include <vector>
#include <functional>
#include "../Utility/DirectXInclude.h"
#include "../Game/GameObject/Transform.h"

class Collider {
public:
	Collider() = default;
	Collider(std::vector<DirectX::XMFLOAT3>& positions);
	Collider(DirectX::BoundingBox box);
public:
	bool GetActiveState() const;

	void UpdateBox(SimpleMath::Matrix& world); 

	SimpleMath::Vector3 GetExtents() const;
	bool CheckCollision(Collider& other);
private:
	DirectX::BoundingOrientedBox mOrigin{};
	DirectX::BoundingOrientedBox mWorld{};

	bool mActive{ false };
};