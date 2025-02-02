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
	const SimpleMath::Vector3& GetPosition() const;
	SimpleMath::Vector3& GetPosition();

	const SimpleMath::Quaternion& GetRotation() const;
	SimpleMath::Quaternion& GetRotation();

	const SimpleMath::Vector3& GetScale() const;
	SimpleMath::Vector3& GetScale();
public:

private:
	SimpleMath::Vector3 mPosition{ SimpleMath::Vector3::Zero };
	SimpleMath::Quaternion mRotation{ SimpleMath::Quaternion::Identity };
	SimpleMath::Vector3 mScale{ SimpleMath::Vector3::One };
	
	Transform* mParent{};

	std::vector<Transform*> mChildren{};
};