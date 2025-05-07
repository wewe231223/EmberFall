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

uint8_t Inventory::AcquireItem(ItemTag tag) {
    uint8_t retIdx{ 0xFF };
    for (uint8_t idx{ 0 }; auto& item : mItems) {
        if (ItemTag::ITEM_NONE == item) {
            item = tag;
            retIdx = idx;
            break;
        }

        ++idx;
    }

    return retIdx;
}

void Inventory::UseItem(std::shared_ptr<GameObject> obj) {
    if (nullptr == obj or mItems.empty()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Use Item Failure - Item doesn't exist");
        return;
    }

    uint8_t itemIdx{ 0xFF };
    ItemTag item = ItemTag::ITEM_NONE;
    for (uint8_t idx{ 0 }; auto& tag : mItems) {
        if (ItemTag::ITEM_NONE != tag) {
            item = tag;
            itemIdx = idx;
            tag = ItemTag::ITEM_NONE;
            break;
        }

        ++idx;
    }

    switch (item) {
    case ItemTag::ITEM_POTION:
    {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Use Item Failure - potion");
        auto buffSystem = obj->GetBuffSystem();
        if (nullptr == buffSystem) {
            return;
        }
        buffSystem->AddBuff<BuffHealScript>();

        auto sessionId = static_cast<SessionIdType>(obj->GetId());
        auto packetUse = FbsPacketFactory::UseItemSC(sessionId, itemIdx);
        gServerCore->Send(sessionId, packetUse);
        break;
    }

    default:
        break;
    }
}