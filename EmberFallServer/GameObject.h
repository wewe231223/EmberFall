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

class IServerGameScene;

enum class ObjectTag {
    PLAYER, 
    MONSTER,
    CORRUPTED_GEM,
    NONE,
};

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject(std::shared_ptr<IServerGameScene> gameScene);
    ~GameObject();
    
public:
    bool IsActive() const;
    NetworkObjectIdType GetId() const;

    std::shared_ptr<Transform> GetTransform() const;
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<Collider> GetCollider() const;

    std::shared_ptr<IServerGameScene> GetOwnGameScene() const;

    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetScale() const;
    SimpleMath::Matrix GetWorld() const;
    ObjectTag GetTag() const;
    SimpleMath::Vector3 GetColor() const;

    void InitId(NetworkObjectIdType id);
    void SetActive(bool active);
    void SetColor(const SimpleMath::Vector3& color);

    void Init();

    void Update(const float deltaTime);
    void LateUpdate(const float deltaTime);

    void OnCollision(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionTerrain(const float height);

    virtual void DispatchGameEvent(GameEvent* event);

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

private:
    void OnCollisionEnter(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionStay(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionExit(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);

private:
    bool mActive{ true };
    ObjectTag mTag{ ObjectTag::NONE };

    NetworkObjectIdType mId{ INVALID_SESSION_ID };                      // network id
    SimpleMath::Vector3 mColor{ SimpleMath::Vector3::One };             // for detecting collision

    std::shared_ptr<Transform> mTransform{ };                           // Transform
    std::shared_ptr<class Physics> mPhysics{ };                         // Physics
    std::shared_ptr<Collider> mCollider{ nullptr };                     // 
    std::vector<std::shared_ptr<GameObjectComponent>> mComponents{ };   // Components

    std::shared_ptr<IServerGameScene> mGameScene{ };
};