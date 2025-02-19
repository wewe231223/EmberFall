#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner } { }

MonsterScript::~MonsterScript() { }

void MonsterScript::Update(const float deltaTime) {
    if (mHp < 0.0f) {
        GetOwner()->SetActive(false);
    }

    int rand = Random::GetRandom<INT32>(0, 1000);
    if (0 == rand % 100) {
        mMoveDir = Random::GetRandomDirVec3();
        mMoveDir.y = 0.0f;
    }

    auto physics = GetOwner()->GetPhysics();
    physics->Acceleration(mMoveDir, deltaTime);
}

void MonsterScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }
