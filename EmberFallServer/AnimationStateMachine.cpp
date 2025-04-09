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

float AnimationStateMachine::GetDuration(AnimationState state) const {
    return mAnimationInfo[static_cast<size_t>(state)].duration;
}

float AnimationStateMachine::GetRemainDuration() const {
    return mCurrState.duration - mAnimationCounter;
}

void AnimationStateMachine::SetOwner(std::shared_ptr<class GameObject> owner) {
    mOwner = owner;
}

void AnimationStateMachine::SetDefaultState(AnimationState state) {
    mDefaultState = mAnimationInfo[static_cast<size_t>(state)];
}

void AnimationStateMachine::ChangeState(AnimationState nextState, bool force) {
    if (false == force and mCurrState.state == nextState) {
        return;
    }

    auto packet = GetPacket<PacketSC::PacketAnimationState>(
        INVALID_SESSION_ID,
        mOwner->GetId(),
        mCurrState.state
    );
    if (force) {
        packet.animState = AnimationState::IDLE;
        gServerCore->SendAll(&packet);
    }

    mCurrState = mAnimationInfo[static_cast<size_t>(nextState)];

    mAnimationChangable = mCurrState.loop;
    mAnimationCounter = 0.0f;
    packet.animState = mCurrState.state;

    gServerCore->SendAll(&packet);
}

void AnimationStateMachine::Update(const float deltaTime) {
    if (true == mCurrState.loop) {
        return;
    }

    mAnimationCounter += deltaTime;
    if (mAnimationCounter > mCurrState.duration) {
        if (mAnimationChangable) {
            ChangeState(mDefaultState.state);
        }
        mAnimationChangable = true;
    }
}
