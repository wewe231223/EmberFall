#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Script.h
// 
// 2025 - 02 - 10 : Script 정의
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameObjectComponent.h"

class GameObject;

enum class ScriptType : uint8_t {
    PLAYER,
    BOSSPLAYER,
    MONSTER,
    CORRUPTED_GEM,
    TRIGGER,
    EVENT_TRIGGER,
    SKILL,
    ITEM
};

class Script abstract : public GameObjectComponent {
public:
    Script(std::shared_ptr<GameObject> owner, ObjectTag tag, ScriptType type);
    virtual ~Script();

public:
    std::shared_ptr<GameObject> GetOwner() const;

    virtual void Init() abstract;

    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) abstract;

    virtual void OnCollisionTerrain(const float height) abstract;

    virtual void DispatchGameEvent(struct GameEvent* event) abstract;
    
private:
    ScriptType mType{ };
    std::weak_ptr<GameObject> mOwner;
};
