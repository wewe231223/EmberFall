#include "pch.h"
#include "BuffScript.h"

BuffScript::BuffScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::NONE, ScriptType::SKILL } { }

BuffScript::~BuffScript() { }

void BuffScript::Init() { }

void BuffScript::Update(const float deltaTime) { }

void BuffScript::LateUpdate(const float deltaTime) { }

void BuffScript::OnAttacked(const float damage, const SimpleMath::Vector3& knockbackForce)
{
}

void BuffScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void BuffScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void BuffScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void BuffScript::OnCollisionTerrain(const float height) { }

void BuffScript::DispatchGameEvent(GameEvent* event) { }
