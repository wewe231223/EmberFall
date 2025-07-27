#include "pch.h"
#include "ArrowScript.h"
#include "GameObject.h"
#include "ServerFrame.h"

ArrowScript::ArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) 
    : Script{ owner, ObjectTag::ARROW, ScriptType::PROJECTILE } {
    owner->mSpec.entity = Packets::EntityType_PROJECTILE_ARROW;
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
    spec.entity = Packets::EntityType_PROJECTILE_ARROW;
    spec.defence = 0.0f;
    spec.damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    spec.hp = GameProtocol::Logic::MAX_HP;

    owner->GetPhysics()->ResizeVelocity(GameProtocol::Unit::ARROW_SPEED.Count());
    owner->GetPhysics()->mFactor.maxMoveSpeed = GameProtocol::Unit::ARROW_SPEED;
    owner->GetPhysics()->mFactor.friction = 0.0f;
    owner->GetPhysics()->mFactor.mass = 3.0f;
    owner->GetPhysics()->SetGravityActive(false);
}

void ArrowScript::Update(const float deltaTime) { 
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    auto moveDir = owner->GetPhysics()->GetMoveDir();
    owner->GetTransform()->SetLook(moveDir);

    auto pos = owner->GetPosition();
}

void ArrowScript::LateUpdate(const float deltaTime) { }

void ArrowScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    auto opponentTag = opponent->GetTag();
    if (ObjectTag::PLAYER == opponentTag or ObjectTag::TRIGGER == opponentTag or ObjectTag::NONE == opponentTag) {
        return;
    }

    auto event = GameEventFactory::GetEvent<AttackEvent>(owner->GetId(), opponent->GetId(), GameProtocol::Logic::DEFAULT_DAMAGE);
    opponent->DispatchGameEvent(event);

    bool expectedRemoved = false;
    if (false == owner->mRemoved.compare_exchange_strong(expectedRemoved, true)) {
        return;
    }
    gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());
}

void ArrowScript::OnCollisionTerrain(const float height) { 
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    owner->GetPhysics()->ResizeVelocity(0.0f);

    bool expectedRemoved = false;
    if (false == owner->mRemoved.compare_exchange_strong(expectedRemoved, true)) {
        return;
    }

    gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());
}

void ArrowScript::DispatchGameEvent(GameEvent* event) { }
