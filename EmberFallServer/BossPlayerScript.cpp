#include "pch.h"
#include "BossPlayerScript.h"
#include "Input.h"
#include "ObjectSpawner.h"

BossPlayerScript::BossPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input)
    : Script{ owner, ObjectTag::PLAYER }, mInput{ input }, mViewList{ static_cast<SessionIdType>(owner->GetId()) } {  
    GetOwner()->SetEntityType(EntityType::BOSS);
}

BossPlayerScript::~BossPlayerScript() { }

void BossPlayerScript::ResetGameScene(std::shared_ptr<IServerGameScene> gameScene) {
    mGameScene = gameScene;
    mViewList.mCurrentScene = gameScene;
}

std::shared_ptr<IServerGameScene> BossPlayerScript::GetCurrentScene() const {
    return mGameScene;
}

void BossPlayerScript::Init() {
    mInteractionTrigger = gObjectSpawner->SpawnTrigger(std::numeric_limits<float>::max(), GetOwner()->GetPosition(), SimpleMath::Vector3{ 15.0f });
}

void BossPlayerScript::Update(const float deltaTime) {
    decltype(auto) owner = GetOwner();

    //mInteractionTrigger->GetTransform()->SetPosition(owner->GetPosition());
    mViewList.mPosition = owner->GetPosition();
    mViewList.Update();
    mViewList.Send();

    CheckAndJump(deltaTime);
    CheckAndMove(deltaTime);

    // Change Weapon
    if (mInput->IsUp('1')) {
        owner->ChangeWeapon(Weapon::NONE);
    }

    if (mInput->IsUp('2')) {
        owner->ChangeWeapon(Weapon::SWORD);
    }

    if (mInput->IsUp('3')) {
        owner->ChangeWeapon(Weapon::SPEAR);
    }

    if (mInput->IsUp('4')) {
        owner->ChangeWeapon(Weapon::BOW);
    }

    // Attack
    if (mInput->IsUp('P')) {
        owner->mAnimationStateMachine.ChangeState(AnimationState::ATTACK);
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
    if (AnimationState::JUMP == GetOwner()->mAnimationStateMachine.GetCurrState()) {
        GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::IDLE);
    }
}

void BossPlayerScript::DispatchGameEvent(GameEvent* event) {
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            GetOwner()->ReduceHealth(attackEvent->damage);
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[] Attacked!!", GetOwner()->GetId());
        }
        break;
    
    default:
        break;
    }
}

void BossPlayerScript::CheckAndMove(const float deltaTime) {
    auto currState = GetOwner()->mAnimationStateMachine.GetCurrState();
    if (AnimationState::MOVE_RIGHT < currState) {
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

    AnimationState changeState{ AnimationState::IDLE };
    if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = 1.5mps;
        changeState = moveDir.x > 0.0f ? AnimationState::MOVE_LEFT : AnimationState::MOVE_RIGHT;
    }

    if (not MathUtil::IsZero(moveDir.z)) {
        if (moveDir.z > 0.0f) {
            physics->mFactor.maxMoveSpeed = 1.5mps;
            changeState = AnimationState::MOVE_BACKWARD;
        }
        else {
            physics->mFactor.maxMoveSpeed = 3.3mps;
            changeState = AnimationState::MOVE_FORWARD;
        }
    }

    GetOwner()->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, GetOwner()->GetTransform()->GetRotation());
    physics->Acceleration(moveDir, deltaTime);
}

void BossPlayerScript::CheckAndJump(const float deltaTime) {
    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsDown(VK_SPACE) and physics->IsOnGround()) {
        GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::JUMP);
        physics->CheckAndJump(deltaTime);
    }
}
