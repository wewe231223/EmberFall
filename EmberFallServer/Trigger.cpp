#include "pch.h"
#include "Trigger.h"
#include "Collider.h"
#include "GameEventManager.h"
#include "GameTimer.h"
#include "GameEventFactory.h"
#include "GameObject.h"

Trigger::Trigger(std::shared_ptr<GameObject> owner, float lifeTime)
    : Script{ owner, ObjectTag::TRIGGER, ScriptType::TRIGGER }, mLifeTime{ lifeTime } {  }

Trigger::~Trigger() { }

std::unordered_set<NetworkObjectIdType>& Trigger::GetObjects() {
    return mInTriggerObjects;
}

void Trigger::Init() {
    if (MathUtil::IsMax(mLifeTime)) {
        return;
    }
}

void Trigger::Update(const float deltaTime) { }

void Trigger::LateUpdate(const float deltaTime) { }

void Trigger::OnCollisionTerrain(const float height) { }

void Trigger::DispatchGameEvent(GameEvent* event) { }
