#pragma once 
#include "../Utility/DirectXInclude.h"

class Transform {
public:
	Transform() = default;
	Transform(const SimpleMath::Vector3&);
	Transform(const SimpleMath::Vector3&, const SimpleMath::Quaternion&);
	Transform(const SimpleMath::Vector3&, const SimpleMath::Quaternion&, const SimpleMath::Vector3&);

	~Transform() = default;

	Transform(const Transform&) = default;
	Transform& operator=(const Transform&) = default;

	Transform(Transform&&) = default;
	Transform& operator=(Transform&&) = default;

public:
	const SimpleMath::Vector3& GetPosition() const;
	const SimpleMath::Quaternion& GetRotation() const;
	const SimpleMath::Vector3& GetScale() const;
	const SimpleMath::Matrix& GetWorldMatrix() const;

	const SimpleMath::Vector3 GetForward() const;
	const SimpleMath::Vector3 GetRight() const;
	const SimpleMath::Vector3 GetUp() const;
	
	const Transform& GetChild(int index) const;

	SimpleMath::Vector3& GetPosition();
	SimpleMath::Quaternion& GetRotation();
	SimpleMath::Vector3& GetScale();
	SimpleMath::Matrix& GetWorldMatrix();
	
	Transform& GetChild(int index);

public:
	void Translate(const SimpleMath::Vector3&);
	void SetPosition(const SimpleMath::Vector3&);

	void Scaling(const SimpleMath::Vector3&);
	void Scaling(float x = 1.f, float y = 1.f, float z = 1.f);

	void Rotate(float pitch = 0.f, float yaw = 0.f, float roll = 0.f);

	void Look(const Transform& target);
	void Look(const SimpleMath::Vector3& target);

	Transform CreateChild(const SimpleMath::Vector3& localPosition = SimpleMath::Vector3::Zero, const SimpleMath::Quaternion& localRotate = SimpleMath::Quaternion::Identity, const SimpleMath::Vector3& localScale = SimpleMath::Vector3::One);

	void SetLocalTransform(const SimpleMath::Matrix& localMatrix);

	void UpdateWorldMatrix();
	void UpdateWorldMatrix(SimpleMath::Matrix& parent);
private:
	SimpleMath::Vector3 mPosition{ DirectX::SimpleMath::Vector3::Zero };
	SimpleMath::Quaternion mRotation{ DirectX::SimpleMath::Quaternion::Identity };
	SimpleMath::Vector3 mScale{ DirectX::SimpleMath::Vector3::One };

	SimpleMath::Matrix mWorldMatrix{ DirectX::SimpleMath::Matrix::Identity };
	SimpleMath::Matrix mLocalMatrix{ DirectX::SimpleMath::Matrix::Identity };

	Transform* mParent{ nullptr };
	std::vector<Transform*> mChildren{};
};