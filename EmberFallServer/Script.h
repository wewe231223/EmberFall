#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Script.h
// 
// 2025 - 02 - 10 : Script 정의
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameObject.h"
#include "GameObjectComponent.h"

class GameObject;

class Script abstract : public GameObjectComponent {
public:
    Script(std::shared_ptr<GameObject> owner, ObjectTag tag);
    virtual ~Script();

public:
    std::shared_ptr<GameObject> GetOwner() const;

    virtual void Init() abstract;

    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) abstract;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;

    virtual void OnCollisionTerrain(const float height) abstract;

    virtual void DispatchGameEvent(struct GameEvent* event) abstract;
    
private:
    std::weak_ptr<GameObject> mOwner;
};
