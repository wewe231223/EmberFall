#include "pch.h"
#include "Item.h"
#include "GameObject.h"

Item::Item() { }

Item::~Item() { }

bool Item::IsActive() const {
    return mActive;
}

void Item::SetActive(bool active) {
    mActive = active;
}

Potion::Potion() { }

Potion::~Potion() { }

void Potion::UseItem(const std::shared_ptr<class GameObject>& obj) {
    obj->RestoreHealth(10.0f);
}
