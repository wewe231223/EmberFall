#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.h
// 
// 2025 - 02 - 02 김성준 : GameObject
// 
//                      : Player가 Session을 상속하도록 하면 더이상 ID는 필요 X
//                      : GameObject 동기화또한 고정배열에서의 인덱스를 ID로 사용하게 할것.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Collider.h"
#include "GameObjectComponent.h"

inline constexpr BYTE MAX_INPUT_STORED = 100;
inline constexpr float TEMP_SPEED = 20.0f;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject();
    ~GameObject();
    
public:
    void SetInput(Key key);
    void Update(const float deltaTime);

    bool IsActive() const;
    void InitId(NetworkObjectIdType id);
    NetworkObjectIdType GetId() const;

    std::shared_ptr<Transform> GetTransform() const;
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<Collider> GetCollider() const;

    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetScale() const;
    SimpleMath::Matrix GetWorld() const;

    void SetColor(const SimpleMath::Vector3& color);
    SimpleMath::Vector3 GetColor() const;

    template <typename ColliderType, typename... Args> 
        requires std::derived_from<ColliderType, Collider> and std::is_constructible_v<ColliderType, Args...>
    void MakeCollider(Args&&... args) {
        mCollider = std::make_shared<ColliderType>(args...);
        mCollider->SetTransform(mTransform);
    }

    void OnCollision(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionTerrain(const float height);

private:
    void OnCollisionEnter(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);

private:
    bool mActive{ true };

    SimpleMath::Vector3 mColor{ SimpleMath::Vector3::One }; // for detecting collision

    NetworkObjectIdType mId{ INVALID_SESSION_ID };          // network id
    std::shared_ptr<class Input> mInput{ };                 // input

    std::shared_ptr<Transform> mTransform{ };               // Transform

    std::shared_ptr<Collider> mCollider{ nullptr };         // collision 
    std::shared_ptr<class Physics> mPhysics{ };             // Physics Test
};