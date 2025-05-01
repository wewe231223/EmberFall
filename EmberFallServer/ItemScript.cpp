#include "pch.h"
#include "ItemScript.h"
#include "GameObject.h"

ItemScript::ItemScript(std::shared_ptr<GameObject> owner, ItemTag item) 
    : Script{ owner, ObjectTag::ITEM, ScriptType::ITEM }, mItemTag{ item } { 
    owner->mSpec.interactable = true;
}

ItemScript::~ItemScript() { }

void ItemScript::Init() { }

void ItemScript::Update(const float deltaTime) { }

void ItemScript::LateUpdate(const float deltaTime) { }

void ItemScript::OnCollisionTerrain(const float height) { }

void ItemScript::DispatchGameEvent(GameEvent* event) { }