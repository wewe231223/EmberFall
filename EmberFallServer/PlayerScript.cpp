#include "pch.h"
#include "PlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "ServerGameScene.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, std::shared_ptr<SessionManager> sessionManager)
    : Script{ owner }, mInput{ input }, mViewList{ static_cast<SessionIdType>(owner->GetId()), sessionManager } {}

PlayerScript::~PlayerScript() { }

void PlayerScript::ResetGameScene(std::shared_ptr<IServerGameScene> gameScene) {
    mGameScene = gameScene;
    mViewList.mCurrentScene = gameScene;
}

std::shared_ptr<IServerGameScene> PlayerScript::GetCurrentScene() const {
    return mGameScene;
}

void PlayerScript::Update(const float deltaTime) {
    mViewList.mPosition = GetOwner()->GetPosition();
    mViewList.Update();
    mViewList.Send();

    auto physics = GetPhysics();

    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    if (Key::DOWN == mInput->GetState('A')) {
        moveDir.x -= 1.0f;
    }

    if (Key::DOWN == mInput->GetState('D')) {
        moveDir.x += 1.0f;
    }

    if (Key::DOWN == mInput->GetState('W')) {
        moveDir.z -= 1.0f;
    }

    if (Key::DOWN == mInput->GetState('S')) {
        moveDir.z += 1.0f;
    }

    if (Key::DOWN == mInput->GetState(VK_SPACE)) {
        physics->Jump(deltaTime);
    }

    if (Key::DOWN == mInput->GetState(VK_F1)) {
        physics->mFactor.mass += 10.0f;
    }

    if (Key::DOWN == mInput->GetState(VK_F2)) {
        physics->mFactor.acceleration += 1.0f;
    }

    moveDir.Normalize();
    physics->Acceleration(moveDir, deltaTime);
}

void PlayerScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }
