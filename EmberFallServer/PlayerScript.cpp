#include "pch.h"
#include "PlayerScript.h"

#include "Input.h"

#include "GameObject.h"
#include "GameSession.h"
#include "ServerFrame.h"

#include "GameRoom.h"
#include "Sector.h"
#include "ObjectManager.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, ObjectTag tag, ScriptType scType)
    : Script{ owner, tag, scType }, mInput{ input } { }

PlayerScript::~PlayerScript() { }

std::shared_ptr<Input> PlayerScript::GetInput() const {
    return mInput;
}

ViewList& PlayerScript::GetViewList() {
    return mViewList;
}

void PlayerScript::Suicide() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->mSpec.hp = 0.0f;
}
