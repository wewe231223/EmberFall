#include "pch.h"
#include "BossPlayerScript.h"

#include "ServerFrame.h"
#include "GameRoom.h"
#include "GameSession.h"
#include "Input.h"
#include "GameObject.h"
#include "HumanPlayerScript.h"

BossPlayerScript::BossPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input)
    : PlayerScript{ owner, input, ObjectTag::BOSSPLAYER, ScriptType::BOSSPLAYER } {  
    auto myOwner = GetOwner();
    if (nullptr == myOwner) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Script Constructor - Owner is Null");
        Crash("Nullptr");
    }

    myOwner->mSpec.entity = Packets::EntityType_BOSS;
}

BossPlayerScript::~BossPlayerScript() { }

void BossPlayerScript::Init() { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = true;
    spec.interactable = false;
    spec.animated = true;

    spec.damage = 30.0f;
    spec.defence = 0.0f;
    owner->mSpec.hp = GameProtocol::Logic::MAX_HP;
}

void BossPlayerScript::Update(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (mInput->IsDown(VK_F1)) {
        mSuperMode = !mSuperMode;
    }

    CheckAndJump(deltaTime);
    CheckAndMove(deltaTime);

    mInput->Update();

    auto ownerRoom = owner->GetMyRoomIdx();
    gGameRoomManager->GetRoom(ownerRoom)->GetStage().UpdatePlayerViewList(owner, owner->GetPosition(), GetViewList().mViewRange.Count());
}

void BossPlayerScript::LateUpdate(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (owner->mSpec.hp > MathUtil::EPSILON) {
        return;
    }

    auto isDead = owner->IsDead();
    if (not isDead) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_DEAD);
    }

    if (isDead and owner->mAnimationStateMachine.GetRemainDuration() <= 0.0f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Boss Player Remove");
        auto executionTime = SysClock::now();
        gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), owner->GetId(), SysClock::now(), TimerEventType::REMOVE_NPC);
        owner->mSpec.active = false;
        return;
    }
}

void BossPlayerScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto tag = opponent->GetTag();
    if (ObjectTag::TRIGGER == tag or ObjectTag::ITEM == tag) {
        return;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->GetPhysics()->SolvePenetration(impulse);
}

void BossPlayerScript::OnCollisionTerrain(const float height) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (Packets::AnimationState_JUMP == owner->mAnimationStateMachine.GetCurrState()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    }
}

void BossPlayerScript::DoInteraction(const std::shared_ptr<GameObject>& target) { }

void BossPlayerScript::DispatchGameEvent(GameEvent* event) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    auto sender = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(event->sender);
    if (nullptr == sender) {
        return;
    }

    auto senderTag = sender->GetTag();
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
    {
        if (event->sender == event->receiver and ObjectTag::MONSTER == senderTag) {
            break;
        }

        if (ObjectTag::PLAYER == senderTag) {
            auto player = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetPlayer(event->sender);
            if (nullptr == player or false == player->mSpec.active) {
#if defined(PRINT_DEBUG_LOG)
                gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Cannot Attack Boss!!! - nullptr or not activated");
#endif
                break;
            }
            
            auto humanScript = player->GetScript<HumanPlayerScript>();
            if (nullptr == humanScript or false == humanScript->IsAttackableBoss()) {
#if defined(PRINT_DEBUG_LOG)
                gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Cannot Attack Boss!!!");
#endif
                break;
            }
        }

        auto attackEvent = reinterpret_cast<AttackEvent*>(event);
        owner->mSpec.hp -= attackEvent->damage;
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACKED, true);
        owner->GetPhysics()->AddForce(attackEvent->knockBackForce);

        auto packetAttacked = FbsPacketFactory::ObjectAttackedSC(owner->GetId(), owner->mSpec.hp);
        owner->StorePacket(packetAttacked);
        break;
    }

    case GameEventType::DESTROY_GEM_COMPLETE:
    {
        break;
    }

    default:
        break;
    }
}

void BossPlayerScript::CheckAndMove(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if (Packets::AnimationState_MOVE_RIGHT < currState) {
        return;
    }

    auto physics{ GetPhysics() };

    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    for (const auto& [key, dir] : GameProtocol::Key::KEY_MOVE_DIR) {
        if (mInput->IsActiveKey(key)) {
            moveDir += dir;
        }
    }

    Packets::AnimationState changeState{ Packets::AnimationState_IDLE };
    if (not MathUtil::IsZero(moveDir.z)) {
        if (not mSuperMode) {
            if (moveDir.z > 0.0f) {
                physics->mFactor.maxMoveSpeed = GameProtocol::Unit::BOSS_PLAYER_WALK_SPEED;
                changeState = Packets::AnimationState_MOVE_BACKWARD;
            }
            else {
                physics->mFactor.maxMoveSpeed = GameProtocol::Unit::BOSS_PLAYER_RUN_SPEED;
                changeState = Packets::AnimationState_MOVE_FORWARD;
            }
        }
        else {
            if (moveDir.z > 0.0f) {
                physics->mFactor.maxMoveSpeed = mSuperSpeed;
                changeState = Packets::AnimationState_MOVE_BACKWARD;
            }
            else {
                physics->mFactor.maxMoveSpeed = mSuperSpeed;
                changeState = Packets::AnimationState_MOVE_FORWARD;
            }
        }
    }
    else if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = GameProtocol::Unit::PLAYER_WALK_SPEED;
        changeState = moveDir.x > 0.0f ? Packets::AnimationState_MOVE_LEFT : Packets::AnimationState_MOVE_RIGHT;
    }

    owner->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, owner->GetTransform()->GetRotation());
    physics->Accelerate(moveDir, owner->GetDeltaTime());

}

void BossPlayerScript::CheckAndJump(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsActiveKey(GameProtocol::Key::KEY_JUMP) and physics->IsOnGround()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_JUMP);
        physics->CheckAndJump(deltaTime);
    }
}
