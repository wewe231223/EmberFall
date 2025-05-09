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

#include "../ServerLib/INetworkObject.h"

#include "GameTimer.h"
#include "Collider.h"
#include "Script.h"
#include "WeaponSystem.h"
#include "AnimationStateMachine.h"
#include "BuffSystem.h"

struct ObjectSpec {
    bool active;                    // 활성화 여부
    bool moveable;                  // 이동 가능 여부
    bool interactable;              // 상호작용 가능 여부
    Packets::EntityType entity;     // 외형 정보
    GameStage stage;                // 스테이지 정보

    float defence;                  // 방어력
    float damage;                   // 공격력

    float hp;                       // 체력
};

class GameObject : public INetworkObject {
public:
    GameObject();
    GameObject(uint16_t roomIdx);
    ~GameObject();

public:
    ObjectTag GetTag() const;

    std::shared_ptr<Transform> GetTransform() const;
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<BoundingObject> GetBoundingObject() const;
    std::shared_ptr<Script> GetScript() const;
    std::shared_ptr<BuffSystem> GetBuffSystem() const;

    SimpleMath::Vector3 GetPrevPosition() const;
    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetEulerRotation() const;
    SimpleMath::Vector3 GetScale() const;
    SimpleMath::Matrix GetWorld() const;

    float GetDeltaTime() const;
    float GetSpeed() const;
    SimpleMath::Vector3 GetMoveDir() const;

    bool IsDead() const;

    // Setter
    void SetTag(ObjectTag tag);

    void DisablePhysics();
    void ChangeWeapon(Packets::Weapon weapon);

    void Reset();
    
    // Initialize Function
    void Init();

    // Update & Process Event Functions
    void RegisterUpdate();
    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;

    void Update();
    void LateUpdate();

    void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    void OnCollisionTerrain(const float height);

    void DoInteraction(std::shared_ptr<GameObject>& obj);

    void DispatchGameEvent(std::shared_ptr<GameEvent> event);

    void Attack();
    void Attack(const SimpleMath::Vector3& dir);

    template <typename BoundingObjType, typename... Args>
        requires std::derived_from<BoundingObjType, BoundingObject>
    void CreateBoundingObject(Args&&... args);

    template <typename ScriptType, typename... Args>
        requires std::derived_from<ScriptType, Script>
    void CreateScript(Args&&... args);

    template <typename ScriptType> requires std::derived_from<ScriptType, Script>
    std::shared_ptr<ScriptType> GetScript();

public:
    ObjectSpec mSpec{ };
    AnimationStateMachine mAnimationStateMachine{ };
    WeaponSystem mWeaponSystem{ INVALID_OBJ_ID };

private:
    std::atomic_bool mHeartBeat{ false };
    ObjectTag mTag{ ObjectTag::NONE };
    float mDeltaTime{ };

    std::unique_ptr<SimpleTimer> mTimer{ };                             // own timer
    std::unique_ptr<OverlappedUpdate> mOverlapped{ };                   // for update
    std::shared_ptr<Transform> mTransform{ };                           // Transform
    std::shared_ptr<class Physics> mPhysics{ };                         // Physics
    std::shared_ptr<Script> mEntityScript{ };                           // script
    std::shared_ptr<BoundingObject> mBoundingObject{ };                 // boundingObject
    std::shared_ptr<BuffSystem> mBuffSystem{ };

    Concurrency::concurrent_queue<std::shared_ptr<GameEvent>> mGameEvents{ };
};

// Definition of template functions
template <typename BoundingObjType, typename... Args>
    requires std::derived_from<BoundingObjType, BoundingObject>
void GameObject::CreateBoundingObject(Args&&... args) {
    mBoundingObject = std::make_shared<BoundingObjType>(args...);
}

template<typename ScriptType, typename ...Args>
    requires std::derived_from<ScriptType, Script>
void GameObject::CreateScript(Args&&... args) {
    mEntityScript = std::make_shared<ScriptType>(args...);
}

template<typename ScriptType> requires std::derived_from<ScriptType, Script>
inline std::shared_ptr<ScriptType> GameObject::GetScript() {
    return std::dynamic_pointer_cast<ScriptType>(mEntityScript);
}
