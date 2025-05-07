#include "pch.h"
#include "Script.h"
#include "GameObject.h"

Script::Script(std::shared_ptr<GameObject> owner, ObjectTag tag, ScriptType type)
    : GameObjectComponent{ owner }, mOwner{ owner }, mType{ type } {
    mOwner.lock()->SetTag(tag);
}

Script::~Script() { }

std::shared_ptr<GameObject> Script::GetOwner() const {
    return mOwner.lock();
}

bool Script::IsActive() const {
    return mActive;
}

void Script::SetActive(bool active) {
    mActive = active;
}
