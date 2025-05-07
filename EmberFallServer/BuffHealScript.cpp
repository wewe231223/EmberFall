#include "pch.h"
#include "BuffHealScript.h"
#include "GameObject.h"

BuffHealScript::BuffHealScript(std::shared_ptr<class GameObject> owner, float delay, float duration, float healpoint) 
    : BuffScript{ owner, duration }, mDelay{ delay }, mHealPoint{ healpoint } { }

BuffHealScript::~BuffHealScript() { }

void BuffHealScript::Init() {
    mDelayCounter = SysClock::now();
}

void BuffHealScript::Update(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner or false == IsActive()) {
        SetActive(false);
        return;
    }

    if (owner->IsDead() or false == owner->mSpec.active) {
        SetActive(false);
        return;
    }

    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(SysClock::now() - mDelayCounter).count() / 1000.0f;
    if (mDelay <= elapsedTime) {
        owner->mSpec.hp += mHealPoint;
        std::clamp(owner->mSpec.hp, 0.0f, GameProtocol::Logic::MAX_HP);

        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Heal!! Owner CurrHP: {}", owner->mSpec.hp);
        mDelayCounter = SysClock::now();
        
        auto packetHeal = FbsPacketFactory::BuffHealSC(owner->mSpec.hp);
        gServerCore->Send(static_cast<SessionIdType>(owner->GetId()), packetHeal);
    }
}

void BuffHealScript::LateUpdate(const float deltaTime) {
    BuffScript::LateUpdate(deltaTime);
}

void BuffHealScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void BuffHealScript::OnCollisionTerrain(const float height) { }

void BuffHealScript::DispatchGameEvent(GameEvent* event) { }
