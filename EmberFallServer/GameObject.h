#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.h
// 
// 2025 - 02 - 02 김성준 : GameObject
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Transform.h"

class Collider;

inline constexpr BYTE MAX_INPUT_STORED = 100;
inline constexpr float TEMP_SPEED = 20.0f;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject();
    ~GameObject();
    
public:
    void SetInput(Key key);
    void Update(const float deltaTime);

    void InitId(SessionIdType id);
    SessionIdType GetId() const;
    SimpleMath::Matrix GetWorld() const;
    Transform& GetTransform();

    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetScale() const;

    std::shared_ptr<Collider> GetCollider() const;

    template <typename ColliderType, typename... Args> 
        requires std::derived_from<ColliderType, Collider> and std::is_constructible_v<ColliderType, Args...>
    void MakeCollider(Args&&... args) {
        mCollider = std::make_shared<ColliderType>(args...);
    }

    void OnCollision(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);

private:
    SessionIdType mId{ INVALID_SESSION_ID };     // network id
    std::array<bool, MAX_KEY_SIZE> mKeyState{ }; // input

    Transform mTransform{ };                    // Transform

    std::shared_ptr<Collider> mCollider{ nullptr }; // collision 
};