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


	SimpleMath::Vector3 GetOriginCenter() const { return mOrigin.Center; }

	SimpleMath::Vector3 GetCenter() const;
	SimpleMath::Vector3 GetExtents() const;

	void SetExtents(float x, float y, float z); 
	void SetCenter(float x, float y, float z);

	bool CheckCollision(Collider& other);

	const DirectX::BoundingOrientedBox& GetWorldBox() const;
	DirectX::BoundingOrientedBox& GetWorldBox();
private:
	DirectX::BoundingOrientedBox mOrigin{};
	DirectX::BoundingOrientedBox mWorld{};

	bool mActive{ false };
};