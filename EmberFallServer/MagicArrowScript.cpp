#include "pch.h"
#include "MagicArrowScript.h"
#include "ServerFrame.h"
#include "GameRoom.h"
#include "Stage.h"
#include "ObjectManager.h"

MagicArrowScript::MagicArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir)
    : Script{ owner, ObjectTag::ARROW, ScriptType::PROJECTILE } {
    owner->mSpec.entity = Packets::EntityType_PROJECTILE_MAGIC_ARROW;
    owner->GetPhysics()->AddVelocity(dir);
    owner->GetPhysics()->ResizeVelocity(GameProtocol::Unit::ARROW_SPEED.Count());
}

MagicArrowScript::~MagicArrowScript() { }

void MagicArrowScript::Init() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = true;
    spec.interactable = false;
    spec.animated = false;
    spec.entity = Packets::EntityType_PROJECTILE_MAGIC_ARROW;
    spec.defence = 0.0f;
    spec.damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    spec.hp = GameProtocol::Logic::MAX_HP;

    owner->GetPhysics()->ResizeVelocity(GameProtocol::Unit::ARROW_SPEED.Count());
    owner->GetPhysics()->mFactor.maxMoveSpeed = GameProtocol::Unit::ARROW_SPEED;
    owner->GetPhysics()->mFactor.friction = 0.0f;
    owner->GetPhysics()->mFactor.mass = 3.0f;
    owner->GetPhysics()->SetGravityActive(false);
}

void MagicArrowScript::Update(const float deltaTime) { 
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    auto moveDir = owner->GetPhysics()->GetMoveDir();
    owner->GetTransform()->SetLook(moveDir);

    auto pos = owner->GetPosition();
}

void MagicArrowScript::LateUpdate(const float deltaTime) { }

void MagicArrowScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    if (ObjectTag::PLAYER == opponent->GetTag()) {
        return;
    }

    owner->mSpec.active = false;

    auto attackEvent = GameEventFactory::GetEvent<AttackEvent>(owner->GetId(), opponent->GetId(), GameProtocol::Logic::DEFAULT_DAMAGE);
    opponent->DispatchGameEvent(attackEvent);

    gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());
    auto pos = owner->GetPosition();
    auto dir = owner->GetTransform()->Forward();

    auto event = GameEventFactory::GetEvent<AttackEvent>(owner->GetId(), SYSTEM_ID, 20.0f);
    gGameRoomManager->GetRoom(owner->GetMyRoomIdx())->GetStage().SpawnEventTrigger(pos, GameProtocol::Logic::MAGIC_ARROW_EXPLOSION_EXTENTS, 
        dir, 0.5f, event, 0.5f, 1, ObjectTag::PLAYER);
}

void MagicArrowScript::OnCollisionTerrain(const float height) {
    auto owner = GetOwner();
    if (nullptr == owner or false == owner->mSpec.active) {
        return;
    }

    owner->GetPhysics()->ResizeVelocity(0.0f);
    gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());

    auto pos = owner->GetPosition();
    auto dir = owner->GetTransform()->Forward();

    auto event = GameEventFactory::GetEvent<AttackEvent>(owner->GetId(), SYSTEM_ID, 20.0f);
    gGameRoomManager->GetRoom(owner->GetMyRoomIdx())->GetStage().SpawnEventTrigger(pos, GameProtocol::Logic::MAGIC_ARROW_EXPLOSION_EXTENTS, 
        dir, 0.5f, event, 0.5f, 1, ObjectTag::PLAYER);
}

void MagicArrowScript::DispatchGameEvent(GameEvent* event) { }
