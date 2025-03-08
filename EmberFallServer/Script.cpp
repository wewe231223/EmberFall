#include "pch.h"
#include "Script.h"

Script::Script(std::shared_ptr<GameObject> owner, ObjectTag tag) 
    : GameObjectComponent{ owner }, mOwner { owner } { 
    mOwner.lock()->SetTag(tag);
}

Script::~Script() { }

std::shared_ptr<GameObject> Script::GetOwner() const {
    if (mOwner.expired()) {
        return nullptr;
    }

    return mOwner.lock();
}
