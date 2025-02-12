#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.h
// 
// 2025 - 02 - 02 김성준 : GameObject
//                      : GameObject 동기화또한 고정배열에서의 인덱스를 ID로 사용하게 할것.
// 
//        02 - 10 : Input 삭제 
//                  Script 컴포넌트 추가 - PlayerScript에서 Input 처리.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Collider.h"
#include "GameObjectComponent.h"

enum class ObjectTag {
    PLAYER, 
    OBJECT,
};

inline constexpr BYTE MAX_INPUT_STORED = 100;
inline constexpr float TEMP_SPEED = 20.0f;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject();
    ~GameObject();
    
public:
    bool IsActive() const;
    void SetActive(bool active);

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
    void CreateCollider(Args&&... args) {
        mCollider = std::make_shared<ColliderType>(args...);
        mCollider->SetTransform(mTransform);
    }
    
    template <typename ComponentType, typename... Args>
        requires std::derived_from<ComponentType, GameObjectComponent>
    void CreateComponent(Args&&... args) {
        mComponents.emplace_back(std::make_shared<ComponentType>(args...));
    }

    template <typename ComponentType> requires std::derived_from<ComponentType, GameObjectComponent>
    std::shared_ptr<ComponentType> GetComponent() {
        for (auto& componenet : mComponents) {
            auto ptr = std::dynamic_pointer_cast<ComponentType>(componenet);
            if (nullptr != ptr) {
                return ptr;
            }
        }

        return nullptr;
    }

    void Update(const float deltaTime);

    void OnCollision(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionTerrain(const float height);

private:
    void OnCollisionEnter(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);
    void OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent);

private:
    bool mActive{ true };

    NetworkObjectIdType mId{ INVALID_SESSION_ID };                      // network id
    SimpleMath::Vector3 mColor{ SimpleMath::Vector3::One };             // for detecting collision

    std::shared_ptr<Transform> mTransform{ };                           // Transform
    std::shared_ptr<class Physics> mPhysics{ };                         // Physics Test
    std::vector<std::shared_ptr<GameObjectComponent>> mComponents{ };   // Components

    std::shared_ptr<Collider> mCollider{ nullptr };                     // 
};