#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner } { }

MonsterScript::~MonsterScript() { }

void MonsterScript::Update(const float deltaTime) {
    // 02-22 움직임 잠시 비활성화
    //int rand = Random::GetRandom<INT32>(0, 1000);
    //if (0 == rand % 100) {
    //    mMoveDir = Random::GetRandomDirVec3();
    //    mMoveDir.y = 0.0f;
    //}

    //auto physics = GetOwner()->GetPhysics();
    //physics->Acceleration(mMoveDir, deltaTime);
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    if (mHp <= MathUtil::EPSILON) {
        GetOwner()->SetActive(false);
    }
}

void MonsterScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        mHp -= static_cast<AttackEvent*>(event)->damage;
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster [{}] Handling AttackEvent, HP: {}",
                                                        GetOwner()->GetId() - OBJECT_ID_START, mHp);
        break;

    default:
        break;
    }
}
