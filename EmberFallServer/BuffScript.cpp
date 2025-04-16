#include "pch.h"
#include "BuffScript.h"

BuffScript::BuffScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::NONE, ScriptType::SKILL } { }

BuffScript::~BuffScript() { }

void BuffScript::Init() { }

void BuffScript::Update(const float deltaTime) { }

void BuffScript::LateUpdate(const float deltaTime) { }

void BuffScript::OnAttacked(const float damage, const SimpleMath::Vector3& knockbackForce){}

void BuffScript::OnCollisionTerrain(const float height) { }

void BuffScript::DispatchGameEvent(GameEvent* event) { }
