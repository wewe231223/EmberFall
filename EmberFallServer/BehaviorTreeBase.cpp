#include "pch.h"
#include "BehaviorTreeBase.h"

using namespace BT;


// ---------------------------------------------------
// SequenceNode
SequenceNode::SequenceNode() { }

SequenceNode::~SequenceNode() { }

void SequenceNode::Start() { }

NodeStatus SequenceNode::Update(const float deltaTime) {
    if (mChildren.size() == mCurrentNode) {
        mCurrentNode = 0;
        return NodeStatus::SUCCESS;
    }

    NodeStatus updateResult = mChildren[mCurrentNode]->Update(deltaTime);
    if (NodeStatus::SUCCESS == updateResult) {
        ++mCurrentNode;
        return NodeStatus::RUNNING;
    }
    else if (NodeStatus::FAIL == updateResult) {
        mCurrentNode = 0;
        return NodeStatus::FAIL;
    }

    return updateResult;
}

void SequenceNode::DispatchGameEvent(GameEvent* event) { 
    mChildren[mCurrentNode]->DispatchGameEvent(event);
}
// SequenceNode end
// ---------------------------------------------------

// ---------------------------------------------------
// SelectorNode
SelectorNode::SelectorNode() { }

SelectorNode::~SelectorNode() { }

void SelectorNode::Start() { }

NodeStatus SelectorNode::Update(const float deltaTime) {
    if (mChildren.size() == mCurrentNode) {
        mCurrentNode = 0;
        return NodeStatus::FAIL;
    }

    NodeStatus updateResult = mChildren[mCurrentNode]->Update(deltaTime);
    if (NodeStatus::FAIL == updateResult) {
        ++mCurrentNode;
        return NodeStatus::RUNNING;
    }
    else if (NodeStatus::SUCCESS == updateResult) {
        mCurrentNode = 0;
        return NodeStatus::SUCCESS;
    }

    return updateResult;
}

void SelectorNode::DispatchGameEvent(GameEvent* event) {
    mChildren[mCurrentNode]->DispatchGameEvent(event);
}
// SelectorNode end
// ---------------------------------------------------

// ---------------------------------------------------
// ConditionNode
BT::ConditionNode::ConditionNode(BTUpdateFn&& fn) 
    : mConditionFn{ std::move(fn) } { }

BT::ConditionNode::~ConditionNode() { }

void BT::ConditionNode::Start() { }

NodeStatus BT::ConditionNode::Update(const float deltaTime) {
    return mConditionFn(deltaTime);
}

void BT::ConditionNode::DispatchGameEvent(GameEvent* event) { }
// ConditionNode end
// ---------------------------------------------------

// ---------------------------------------------------
// ActionNode
BT::ActionNode::ActionNode(BTUpdateFn&& fn) 
    : mActionFn{ fn } { }

BT::ActionNode::~ActionNode() { }

void BT::ActionNode::Start() { }

NodeStatus BT::ActionNode::Update(const float deltaTime) {
    return mActionFn(deltaTime);
}

void BT::ActionNode::DispatchGameEvent(GameEvent* event) { }
// ActionNode end
// ---------------------------------------------------

void BT::BehaviorTree::Update(float deltaTime) {
    auto updateResult = mRoot->Update(deltaTime);
    //if (NodeStatus::SUCCESS == updateResult) {
    //    mRoot->Reset();
    //}
}

void BT::BehaviorTree::SetRoot(NodePtr&& rootNode) {
    mRoot = std::move(rootNode);
}
