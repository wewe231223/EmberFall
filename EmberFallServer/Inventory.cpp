#include "pch.h"
#include "Inventory.h"
#include "GameObject.h"
#include "BuffSystem.h"
#include "BuffHealScript.h"

Inventory::Inventory() { }

Inventory::~Inventory() { }

uint8_t Inventory::GetItemCount(ItemTag tag) const {
    return static_cast<uint8_t>(mItems.size());
}

void Inventory::AcquireItem(ItemTag tag) {
    mItems.push(tag);
}

void Inventory::UseItem(std::shared_ptr<GameObject> obj) {
    if (nullptr == obj or mItems.empty()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Use Item Failure - Item doesn't exist");
        return;
    }

    auto item = mItems.front();
    mItems.pop();

    switch (item) {
    case ItemTag::ITEM_POTION:
    {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Use Item Failure - potion");
        auto buffSystem = obj->GetBuffSystem();
        if (nullptr == buffSystem) {
            return;
        }
        buffSystem->AddBuff<BuffHealScript>();
        break;
    }

    default:
        break;
    }
}