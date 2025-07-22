#include "pch.h"
#include "ArrowScript.h"
#include "GameObject.h"
#include "ServerFrame.h"

ArrowScript::ArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos,
    const SimpleMath::Vector3& dir) 
    : Script{ owner, ObjectTag::ARROW, ScriptType::TRIGGER } {
    owner->mSpec.entity = Packets::EntityType_PROJECTILE;
    owner->GetPhysics()->AddVelocity(dir);
    owner->GetPhysics()->ResizeVelocity(GameProtocol::Unit::ARROW_SPEED.Count());
}

ArrowScript::~ArrowScript() { }

void ArrowScript::Init() { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = true;
    spec.interactable = false;
    spec.animated = false;
    spec.entity = Packets::EntityType_PROJECTILE;
    spec.defence = 0.0f;
    spec.damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    spec.hp = GameProtocol::Logic::MAX_HP;

    owner->GetPhysics()->mFactor.maxMoveSpeed = GameProtocol::Unit::ARROW_SPEED;
}

void ArrowScript::Update(const float deltaTime) { }

void ArrowScript::LateUpdate(const float deltaTime) { }

void ArrowScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    owner->mSpec.active = false;

    auto event = GameEventFactory::GetEvent<AttackEvent>(owner->GetId(), opponent->GetId(), GameProtocol::Logic::DEFAULT_DAMAGE);
    opponent->DispatchGameEvent(event);

    gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());
}

void ArrowScript::OnCollisionTerrain(const float height) { }

void ArrowScript::DispatchGameEvent(GameEvent* event) { }
