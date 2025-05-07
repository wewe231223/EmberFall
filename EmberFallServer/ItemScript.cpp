#include "pch.h"
#include "ItemScript.h"
#include "ServerFrame.h"
#include "GameObject.h"

ItemScript::ItemScript(std::shared_ptr<GameObject> owner, ItemTag item) 
    : Script{ owner, ObjectTag::ITEM, ScriptType::ITEM }, mItemTag{ item } { }

ItemScript::~ItemScript() { }

ItemTag ItemScript::GetItemTag() const {
    return mItemTag;
}

void ItemScript::Init() { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->mSpec.active = true;
    owner->mSpec.interactable = true;


    gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), owner->GetId(), SysClock::now() + 10s, TimerEventType::REMOVE_NPC);
}

void ItemScript::Update(const float deltaTime) { }

void ItemScript::LateUpdate(const float deltaTime) { }

void ItemScript::OnCollisionTerrain(const float height) { }

void ItemScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void ItemScript::DispatchGameEvent(GameEvent* event) { }