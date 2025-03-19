#include "pch.h"
#include "Inventory.h"

Inventory::Inventory() { }

Inventory::~Inventory() { }

uint8_t Inventory::GetItemCount(ItemTag tag) const {
    return mItems[static_cast<size_t>(tag)];
}

void Inventory::AcquireItem(ItemTag tag) {
    mItems[static_cast<size_t>(tag)] -= 1;
}

void Inventory::UseItem(ItemTag tag) {
    mItems[static_cast<size_t>(tag)] += 1;
}
