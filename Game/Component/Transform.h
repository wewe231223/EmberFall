#pragma once 
#include "../Game/Component/ComponentBase.h"
#include "../Utility/DirectXInclude.h"

class Transform : public ComponentBase {
public:
	static constexpr size_t index = 0;
public:
	Transform(); 
	virtual ~Transform(); 
public: // Getter 
	const DirectX::SimpleMath::Vector3& GetPosition() const;
	DirectX::SimpleMath::Vector3& GetPosition();

	const DirectX::SimpleMath::Quaternion& GetRotation() const;
	DirectX::SimpleMath::Quaternion& GetRotation();

	const DirectX::SimpleMath::Vector3& GetScale() const;
	DirectX::SimpleMath::Vector3& GetScale();
public:

private:
	DirectX::SimpleMath::Vector3 mPosition{ DirectX::SimpleMath::Vector3::Zero };
	DirectX::SimpleMath::Quaternion mRotation{ DirectX::SimpleMath::Quaternion::Identity };
	DirectX::SimpleMath::Vector3 mScale{ DirectX::SimpleMath::Vector3::One };
	
	Transform* mParent{};

	std::vector<Transform*> mChildren{};
};