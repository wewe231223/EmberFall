#include "pch.h"
#include "BossPlayerScript.h"
#include "Input.h"
#include "ObjectSpawner.h"
#include "GameObject.h"

BossPlayerScript::BossPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input)
    : Script{ owner, ObjectTag::PLAYER, ScriptType::BOSSPLAYER }, mInput{ input }, mViewList{ } {  
    GetOwner()->mSpec.entity = Packets::EntityType_BOSS;
}

BossPlayerScript::~BossPlayerScript() { }

void BossPlayerScript::Init() {
    mInteractionTrigger = gObjectSpawner->SpawnTrigger(std::numeric_limits<float>::max(), GetOwner()->GetPosition(), SimpleMath::Vector3{ 15.0f });
}

void BossPlayerScript::Update(const float deltaTime) {
    decltype(auto) owner = GetOwner();

    CheckAndJump(deltaTime);
    CheckAndMove(deltaTime);

    // Attack
    if (mInput->IsUp('P')) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACK);
        owner->Attack();
    }
}

void BossPlayerScript::LateUpdate(const float deltaTime) {
    mInput->Update();
}

void BossPlayerScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {}

void BossPlayerScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {}

void BossPlayerScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {}

void BossPlayerScript::OnCollisionTerrain(const float height) {
    if (Packets::AnimationState_JUMP == GetOwner()->mAnimationStateMachine.GetCurrState()) {
        GetOwner()->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    }
}

void BossPlayerScript::DispatchGameEvent(GameEvent* event) {
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            GetOwner()->mSpec.hp -= attackEvent->damage;
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[] Attacked!!", GetOwner()->GetId());
        }
        break;
    
    default:
        break;
    }
}

void BossPlayerScript::CheckAndMove(const float deltaTime) {
    auto currState = GetOwner()->mAnimationStateMachine.GetCurrState();
    if (Packets::AnimationState_MOVE_RIGHT < currState) {
        return;
    }

    auto physics{ GetPhysics() };

    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    if (mInput->IsActiveKey('D')) {
        moveDir.x -= 1.0f;
    }

    if (mInput->IsActiveKey('W')) {
        moveDir.z -= 1.0f;
    }

    if (mInput->IsActiveKey('A')) {
        moveDir.x += 1.0f;
    }

    if (mInput->IsActiveKey('S')) {
        moveDir.z += 1.0f;
    }

    Packets::AnimationState changeState{ Packets::AnimationState_IDLE };
    if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = 1.5mps;
        changeState = moveDir.x > 0.0f ? Packets::AnimationState_MOVE_LEFT : Packets::AnimationState_MOVE_RIGHT;
    }

    if (not MathUtil::IsZero(moveDir.z)) {
        if (moveDir.z > 0.0f) {
            physics->mFactor.maxMoveSpeed = 1.5mps;
            changeState = Packets::AnimationState_MOVE_BACKWARD;
        }
        else {
            physics->mFactor.maxMoveSpeed = 3.3mps;
            changeState = Packets::AnimationState_MOVE_FORWARD;
        }
    }

    GetOwner()->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, GetOwner()->GetTransform()->GetRotation());
    physics->Accelerate(moveDir);
}

void BossPlayerScript::CheckAndJump(const float deltaTime) {
    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsDown(VK_SPACE) and physics->IsOnGround()) {
        GetOwner()->mAnimationStateMachine.ChangeState(Packets::AnimationState_JUMP);
        physics->CheckAndJump(deltaTime);
    }
}
