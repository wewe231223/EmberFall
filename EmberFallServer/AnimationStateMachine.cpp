#include "pch.h"
#include "AnimationStateMachine.h"
#include "GameObject.h"

AnimationStateMachine::AnimationStateMachine() { }

AnimationStateMachine::~AnimationStateMachine() { }

AnimationState AnimationStateMachine::GetCurrState() const {
    return mCurrState.state;
}

void AnimationStateMachine::SetOwner(std::shared_ptr<class GameObject> owner) {
    mOwner = owner;
}

void AnimationStateMachine::SetDefaultState(AnimationState state) {
    mDefaultState = mAnimationInfo[static_cast<size_t>(state)];
}

void AnimationStateMachine::ChangeState(AnimationState nextState) {
    if (mCurrState.state == nextState) {
        return;
    }

    mAnimationCounter = 0.0f;
    mCurrState = mAnimationInfo[static_cast<size_t>(nextState)];

    auto packet = GetPacket<PacketSC::PacketAnimationState>(
        INVALID_SESSION_ID,
        mOwner->GetId(),
        mCurrState.state
    );

    gServerCore->SendAll(&packet);
}

void AnimationStateMachine::Update(const float deltaTime) {
    if (true == mCurrState.loop) {
        return;
    }

    mAnimationCounter += deltaTime;
    if (mAnimationCounter > mCurrState.duration) {
        ChangeState(AnimationState::IDLE);
    }
}
