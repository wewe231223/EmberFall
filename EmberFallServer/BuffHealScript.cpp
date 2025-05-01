#include "pch.h"
#include "BuffHealScript.h"
#include "GameObject.h"

BuffHealScript::BuffHealScript(std::shared_ptr<class GameObject> owner, float delay, float duration, float healpoint) 
    : BuffScript{ owner, duration }, mDelay{ delay }, mHealPoint{ healpoint } { }

BuffHealScript::~BuffHealScript() { }

void BuffHealScript::Init() {
}

void BuffHealScript::Update(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (owner->IsDead() or false == owner->mSpec.active) {
        return;
    }

    if (mDelay >= 1.0f) {
        owner->mSpec.hp += 10.0f;
    }
}

void BuffHealScript::LateUpdate(const float deltaTime) {
    BuffScript::LateUpdate(deltaTime);
}

void BuffHealScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void BuffHealScript::OnCollisionTerrain(const float height) { }

void BuffHealScript::DispatchGameEvent(GameEvent* event) { }
