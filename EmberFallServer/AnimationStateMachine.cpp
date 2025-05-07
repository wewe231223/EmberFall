#include "pch.h"
#include "AnimationStateMachine.h"
#include "GameObject.h"

AnimationStateMachine::AnimationStateMachine() { }

AnimationStateMachine::~AnimationStateMachine() { }

void AnimationStateMachine::Init(const std::string& entityKey) {
    mAnimInfo = ResourceManager::GetAnimInfo(entityKey);

    mDefaultState = Packets::AnimationState_IDLE;
    mDefaultAnimInfo = mAnimInfo->states[static_cast<size_t>(mDefaultState)];
}

bool AnimationStateMachine::IsChangable() const {
    return mIsLoopAnimation;
}

Packets::AnimationState AnimationStateMachine::GetCurrState() const {
    return mCurrState;
}

float AnimationStateMachine::GetDuration(Packets::AnimationState state) const {
    return static_cast<float>(mAnimInfo->states[state].duration);
}

float AnimationStateMachine::GetRemainDuration() const {
    return static_cast<float>(mCurrAnimInfo.duration) - mAnimationCounter;
}

void AnimationStateMachine::SetOwner(std::shared_ptr<class GameObject> owner) {
    mOwner = owner;
}

void AnimationStateMachine::SetDefaultState(Packets::AnimationState state) {
    mDefaultState = state;
    mDefaultAnimInfo = mAnimInfo->states[static_cast<size_t>(state)];
}

void AnimationStateMachine::ChangeState(Packets::AnimationState nextState, bool force) {
    if (nullptr == mOwner or nullptr == mAnimInfo or (false == force and mCurrState == nextState)) {
        return;
    }

    auto ownerId = mOwner->GetId();
    mCurrState = nextState;
    mCurrAnimInfo = mAnimInfo->states[static_cast<size_t>(nextState)];

    mIsLoopAnimation = mCurrAnimInfo.loop;
    mAnimationCounter = 0.0f;
    mAnimationChanged = true;

    auto packetAnim = FbsPacketFactory::ObjectAnimationChangedSC(ownerId, mCurrState);
    mOwner->StorePacket(packetAnim);
}

void AnimationStateMachine::Update(const float deltaTime) {
    if (true == mCurrAnimInfo.loop) {
        return;
    }

    mAnimationCounter += deltaTime;
    if (mAnimationCounter < mCurrAnimInfo.duration) {
        return;
    }

    if (not mIsLoopAnimation and mCurrState != Packets::AnimationState_DEAD) {
        ChangeState(mDefaultState);
    }
}
