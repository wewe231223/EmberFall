#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObjectComponent.h
// 
// 2025 - 03 - 03(설명추가) 김성준 : Component Base 클래스
//  
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Physics.h"
#include "Transform.h"

class GameObjectComponent abstract : public std::enable_shared_from_this<GameObjectComponent> {
public:
    GameObjectComponent(const std::shared_ptr<class GameObject>& owner);
    GameObjectComponent(const std::shared_ptr<Physics>& physics=nullptr, const std::shared_ptr<Transform>& transform=nullptr);
    virtual ~GameObjectComponent();

public:
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<Transform> GetTransform() const;

    virtual void Init() abstract;
       
    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) abstract;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    
    virtual void DispatchGameEvent(struct GameEvent* event) abstract;

private:
    std::weak_ptr<Physics> mPhysics{ };
    std::weak_ptr<Transform> mTransform{ };
};