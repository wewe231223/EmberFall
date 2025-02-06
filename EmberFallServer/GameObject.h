#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.h
// 
// 2025 - 02 - 02 김성준 : GameObject
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Collider.h"
#include "Transform.h"

inline constexpr BYTE MAX_INPUT_STORED = 100;
inline constexpr float TEMP_SPEED = 20.0f;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject();
    ~GameObject();
    
public:
    void SetInput(Key key);
    void UpdateInput(const float deltaTime);
    void Update(const float deltaTime);

    void InitId(NetworkObjectIdType id);
    NetworkObjectIdType GetId() const;

    std::shared_ptr<Transform> GetTransform();
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

    void MakePhysics(); // test

    void OnCollision(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionTerrain(const float height);

private:
    void OnCollisionEnter(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);

private:
    SimpleMath::Vector3 mColor{ SimpleMath::Vector3::One }; // for detecting collision

    NetworkObjectIdType mId{ INVALID_SESSION_ID };        // network id
    std::array<bool, MAX_KEY_SIZE> mKeyState{ };    // input

    std::shared_ptr<Transform> mTransform{ };       // Transform

    std::shared_ptr<Collider> mCollider{ nullptr }; // collision 
    std::shared_ptr<class Physics> mPhysics{ };
};