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
#include "WeaponSystem.h"
#include "AnimationStateMachine.h"

class IServerGameScene;

enum class ObjectTag : uint8_t {
    ENV,
    BOSSPLAYER,
    PLAYER, 
    MONSTER,
    CORRUPTED_GEM,
    ITEM,
    TRIGGER,
    ARROW,
    NONE,
};

struct ObjectSpec {
    bool active;                    // 활성화 여부
    bool interactable;              // 상호작용 가능 여부
    bool attackable;                // 공격 가능 여부
    Packets::EntityType entity;     // 외형 정보

    float defence;                  // 방어력
    float damage;                   // 공격력

    float hp;                       // 체력
};

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject();
    ~GameObject();

public:
    // Getter
    bool IsCollidingObject() const;

    ObjectTag GetTag() const;
    NetworkObjectIdType GetId() const;

    std::shared_ptr<Transform> GetTransform() const;
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<Collider> GetCollider() const;

    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetEulerRotation() const;
    SimpleMath::Vector3 GetScale() const;
    SimpleMath::Matrix GetWorld() const;

    float GetSpeed() const;
    SimpleMath::Vector3 GetMoveDir() const;

    // Setter
    void SetTag(ObjectTag tag);

    void SetCollider(std::shared_ptr<Collider> collider);
    void DisablePhysics();
    void ChangeWeapon(Packets::Weapon weapon);

    void ClearComponents();
    void Reset();
    
    // Initialize Function
    void Init();
    void InitId(NetworkObjectIdType id);

    // Update & Process Event Functions
    void Update(const float deltaTime);
    void LateUpdate(const float deltaTime);

    void OnCollision(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionTerrain(const float height);

    void DispatchGameEvent(GameEvent* event);

    void Attack();
    void Attack(const SimpleMath::Vector3& dir);

    // Declaration of template Functions
    template <typename ColliderType, typename... Args>
        requires std::derived_from<ColliderType, Collider> and std::is_constructible_v<ColliderType, Args...>
    void CreateCollider(Args&&... args);

    template <typename ComponentType, typename... Args>
        requires std::derived_from<ComponentType, GameObjectComponent>
    void CreateComponent(Args&&... args);

    template <typename ComponentType> requires std::derived_from<ComponentType, GameObjectComponent>
    std::shared_ptr<ComponentType> GetComponent();

private:
    void OnCollisionEnter(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionStay(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionExit(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);

public:
    AnimationStateMachine mAnimationStateMachine{ };
    ObjectSpec mSpec{ };

private:
    ObjectTag mTag{ ObjectTag::NONE };
    NetworkObjectIdType mId{ INVALID_OBJ_ID };                      // network id

    std::shared_ptr<Transform> mTransform{ };                           // Transform
    std::shared_ptr<class Physics> mPhysics{ };                         // Physics
    std::shared_ptr<Collider> mCollider{ nullptr };                     // 
    std::vector<std::shared_ptr<GameObjectComponent>> mComponents{ };   // Components

    WeaponSystem mWeaponSystem{ INVALID_OBJ_ID };
};

// Definition of template functions
template<typename ColliderType, typename ...Args>
    requires std::derived_from<ColliderType, Collider> and std::is_constructible_v<ColliderType, Args...>
inline void GameObject::CreateCollider(Args && ...args) {
    if (nullptr != mCollider) {
        mCollider.reset();
    }

    mCollider = std::make_shared<ColliderType>(args...);
    mCollider->SetTransform(mTransform);
}

template<typename ComponentType, typename ...Args>
    requires std::derived_from<ComponentType, GameObjectComponent>
inline void GameObject::CreateComponent(Args && ...args) {
    mComponents.emplace_back(std::make_shared<ComponentType>(args...));
}

template<typename ComponentType>
    requires std::derived_from<ComponentType, GameObjectComponent>
inline std::shared_ptr<ComponentType> GameObject::GetComponent() {
    for (auto& componenet : mComponents) {
        auto ptr = std::dynamic_pointer_cast<ComponentType>(componenet);
        if (nullptr != ptr) {
            return ptr;
        }
    }

    return nullptr;
}
