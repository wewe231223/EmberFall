#include "pch.h"
#include "Item.h"
#include "GameObject.h"

void Item::UseItem(ItemTag tag, const std::shared_ptr<class GameObject>& obj) {
    switch (tag) {
    case ItemTag::ITEM_POTION:
        Potion::UseItem(obj);
        break;

    case ItemTag::ITEM_HOLYWATER:
        HolyWater::UseItem(obj);
        break;

    case ItemTag::ITEM_CROSS:
        Cross::UseItem(obj);
        break;

    default:
        break;
    }
}

void Potion::UseItem(const std::shared_ptr<class GameObject>& obj) {
    obj->mSpec.hp += 10.0f;
}

void HolyWater::UseItem(const std::shared_ptr<class GameObject>& obj) {
    
}

void Cross::UseItem(const std::shared_ptr<class GameObject>& obj) {

}

Packets::EntityType ItemTagToEntityType(ItemTag tag) 
{
    switch (tag) {
    case ItemTag::ITEM_POTION:
        return Packets::EntityType_ITEM_POTION;

    case ItemTag::ITEM_CROSS:
        return Packets::EntityType_ITEM_CROSS;

    case ItemTag::ITEM_HOLYWATER:
        return Packets::EntityType_ITEM_HOLYWATER;
    }

    return Packets::EntityType();
}

Packets::ItemType ItemTagToItemType(ItemTag tag)
{
    switch (tag) {
    case ItemTag::ITEM_POTION:
        return Packets::ItemType_POTION;

    case ItemTag::ITEM_CROSS:
        return Packets::ItemType_CROSS;

    case ItemTag::ITEM_HOLYWATER:
        return Packets::ItemType_HOLY_WATER;
    }

    return Packets::ItemType();
}
