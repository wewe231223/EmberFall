#include "pch.h"
#include "ItemScript.h"

ItemScript::ItemScript(std::shared_ptr<GameObject> owner, ItemTag item) 
    : Script{ owner, ObjectTag::ITEM }, mItemTag{ item } { 
    owner->mSpec.interactable = true;
}

ItemScript::~ItemScript() { }

void ItemScript::Init() { }

void ItemScript::Update(const float deltaTime) { }

void ItemScript::LateUpdate(const float deltaTime) { }

void ItemScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::OnCollisionTerrain(const float height) { }

void ItemScript::DispatchGameEvent(GameEvent* event) { }