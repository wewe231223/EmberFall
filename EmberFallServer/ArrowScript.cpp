#include "pch.h"
#include "ArrowScript.h"
#include "GameObject.h"
#include "GameEventManager.h"

ArrowScript::ArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos,
    const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed) 
    : Script{ owner, ObjectTag::ARROW, ScriptType::TRIGGER } {
    owner->mSpec.entity = Packets::EntityType_PROJECTILE;
    owner->GetTransform()->Translate(pos);
    owner->GetPhysics()->AddVelocity(dir);
    owner->GetPhysics()->ResizeVelocity(speed.Count());
}

ArrowScript::~ArrowScript() { }

void ArrowScript::Init() { }

void ArrowScript::Update(const float deltaTime) { }

void ArrowScript::LateUpdate(const float deltaTime) { }

void ArrowScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    gEventManager->PushEvent<AttackEvent>(
        GetOwner()->GetId(),
        opponent->GetId(),
        GameProtocol::Logic::DEFAULT_DAMAGE
    );

    GetOwner()->mSpec.active = false;
}

void ArrowScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ArrowScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ArrowScript::OnCollisionTerrain(const float height) { }

void ArrowScript::DispatchGameEvent(GameEvent* event) { }
