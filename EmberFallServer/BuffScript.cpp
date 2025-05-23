#include "pch.h"
#include "BuffScript.h"

BuffScript::BuffScript(std::shared_ptr<GameObject> owner, float duration) 
    : Script{ owner, ObjectTag::NONE, ScriptType::SKILL }, mDuration{ duration } { }

BuffScript::~BuffScript() { }

Buff BuffScript::GetBuffType() const {
    return mBuff;
}

void BuffScript::LateUpdate(const float deltaTime) {
    mDurationCounter += deltaTime;
    if (mDurationCounter >= mDuration) {
        SetActive(false);
    }
}
