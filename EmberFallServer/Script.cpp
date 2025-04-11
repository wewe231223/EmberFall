#include "pch.h"
#include "Script.h"
#include "GameObject.h"

Script::Script(std::shared_ptr<GameObject> owner, ObjectTag tag, ScriptType type)
    : GameObjectComponent{ owner }, mOwner{ owner }, mType{ type } {
    mOwner.lock()->SetTag(tag);
}

Script::~Script() { }

std::shared_ptr<GameObject> Script::GetOwner() const {
    if (mOwner.expired()) {
        return nullptr;
    }

    return mOwner.lock();
}
