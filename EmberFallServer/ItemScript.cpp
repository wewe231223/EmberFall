#include "pch.h"
#include "ItemScript.h"

ItemScript::ItemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::NONE } { }

ItemScript::~ItemScript() { }

void ItemScript::Init() { }

void ItemScript::Update(const float deltaTime) { }

void ItemScript::LateUpdate(const float deltaTime) { }

void ItemScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    if (ObjectTag::PLAYER == opponent->GetTag()) {
        
    }
}

void ItemScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::DispatchGameEvent(GameEvent* event) { }
