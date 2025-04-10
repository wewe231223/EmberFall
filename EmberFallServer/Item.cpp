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
