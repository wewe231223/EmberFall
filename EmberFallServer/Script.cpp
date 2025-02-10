#include "pch.h"
#include "Script.h"

Script::Script(std::shared_ptr<class GameObject> owner) 
    : GameObjectComponent{ owner }, mOwner { owner } { }

Script::~Script() { }

std::shared_ptr<GameObject> Script::GetOwner() const {
    if (mOwner.expired()) {
        return nullptr;
    }

    return mOwner.lock();
}
