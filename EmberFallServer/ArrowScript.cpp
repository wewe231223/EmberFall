#include "pch.h"
#include "ArrowScript.h"
#include "GameObject.h"

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

void ArrowScript::OnCollisionTerrain(const float height) { }

void ArrowScript::DispatchGameEvent(GameEvent* event) { }
