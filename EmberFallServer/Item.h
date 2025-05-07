#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Item.h
// 
// 2025 - 03 - 11 : Item 효과들에 대한 정의
// 
//                  아이템은 그냥 함수로 만들자
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class ItemTag : uint8_t {
    ITEM_POTION,
    ITEM_HOLYWATER,
    ITEM_CROSS,
    ITEM_TAG_COUNT
};

Packets::EntityType ItemTagToEntityType(ItemTag tag);
Packets::ItemType ItemTagToItemType(ItemTag tag);

using ItemEffectFn = std::function<void(const std::shared_ptr<class GameObject>&)>;

class Item abstract {
public:
    static void UseItem(ItemTag tag, const std::shared_ptr<class GameObject>& obj);
};

class Potion : public Item {
public:
    static void UseItem(const std::shared_ptr<class GameObject>& obj);
};

class HolyWater : public Item {
public:
    static void UseItem(const std::shared_ptr<class GameObject>& obj);
};

class Cross : public Item {
public:
    static void UseItem(const std::shared_ptr<class GameObject>& obj);
};