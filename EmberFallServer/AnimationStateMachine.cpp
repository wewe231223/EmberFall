#include "pch.h"
#include "AnimationStateMachine.h"
#include "GameObject.h"

AnimationStateMachine::AnimationStateMachine() { }

AnimationStateMachine::~AnimationStateMachine() { }

bool AnimationStateMachine::IsChangable() const {
    return mAnimationChangable;
}

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

    mCurrState = mAnimationInfo[static_cast<size_t>(nextState)];

    mAnimationChangable = mCurrState.loop;
    mAnimationCounter = 0.0f;

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
        ChangeState(mDefaultState.state);
        mAnimationChangable = true;
    }
}
