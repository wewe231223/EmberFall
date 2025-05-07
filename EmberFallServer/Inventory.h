#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Inventory.h
// 
// 2025 - 03 - 14 : Inventory 
//                  Player가 획득한 아이템을 여기에 저장해두자
//                  아이템을 저장하고 사용할때 여기서 꺼내 쓰는걸로...
// 
//                  다른 게임의 창고와는 다른 개념으로 쓰는거라서 단순하게 아이템 종류와, 아이템 개수만 기록하고
//                  아이템 개수에 따라 사용가능 여부만을 리턴하도록 하자.
//              
//                  처음에는 unordered_map을 사용했지만 아이템 종류가 정해져있고, 또 그 종류가 너무 적어서 std::array로
//                  관리하는 것이 더 편리할 거 같음.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Item.h"

class Inventory {
public:
    static constexpr size_t MAX_INVENTORY{ 5 };

public:
    Inventory();
    ~Inventory();

public:
    uint8_t GetItemCount(ItemTag tag) const;

    uint8_t AcquireItem(ItemTag tag);
    void UseItem(std::shared_ptr<GameObject> obj);

private:
    std::array<ItemTag, MAX_INVENTORY> mItems{ };
};