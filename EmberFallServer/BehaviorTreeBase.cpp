#include "pch.h"
#include "BehaviorTreeBase.h"

using namespace BT;


// ---------------------------------------------------
// SequenceNode
SequenceNode::SequenceNode() { }

SequenceNode::~SequenceNode() { }

void SequenceNode::Start() { 
    mCurrentNode = 0;
}

NodeStatus SequenceNode::Update(const float deltaTime) {
    if (mChildren.size() == mCurrentNode) {
        mCurrentNode = 0;
        return NodeStatus::SUCCESS;
    }

    NodeStatus updateResult = mChildren[mCurrentNode]->Update(deltaTime);

    switch (updateResult) {
    case NodeStatus::SUCCESS:
        ++mCurrentNode;
        break;

    case NodeStatus::FAIL:
        mCurrentNode = 0;
        break;

    default:
        break;
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

void SelectorNode::Start() { 
    mCurrentNode = 0;
}

NodeStatus SelectorNode::Update(const float deltaTime) {
    if (mChildren.size() == mCurrentNode) {
        mCurrentNode = 0;
        return NodeStatus::FAIL;
    }

    NodeStatus updateResult = mChildren[mCurrentNode]->Update(deltaTime);

    switch (updateResult) {
    case NodeStatus::FAIL:
        ++mCurrentNode;
        break;

    case NodeStatus::SUCCESS:
        mCurrentNode = 0;
        break;

    default:
        break;
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

void BT::BehaviorTree::DispatchGameEvent(GameEvent* event) {
    mRoot->DispatchGameEvent(event);
}

void BT::BehaviorTree::Start() {
    mRoot->Start();
}

void BT::BehaviorTree::Update(float deltaTime) {
    auto updateResult = mRoot->Update(deltaTime);
    //if (NodeStatus::SUCCESS == updateResult) {
    //    mRoot->Reset();
    //}
}

void BT::BehaviorTree::SetRoot(NodePtr&& rootNode) {
    mRoot = std::move(rootNode);
}

void BT::BehaviorTree::SetOtherTree(BehaviorTree& tree) {
    auto sequence = dynamic_cast<SequenceNode*>(mRoot.get());
    auto selector = dynamic_cast<SelectorNode*>(mRoot.get());
    assert(sequence or selector);

    if (sequence) {
        sequence->AddChild(std::move(tree.mRoot));
    } 
    else {
        selector->AddChild(std::move(tree.mRoot));
    }
}

void BT::BehaviorTree::SetChild(NodePtr&& node) {
    auto sequence = dynamic_cast<SequenceNode*>(mRoot.get());
    auto selector = dynamic_cast<SelectorNode*>(mRoot.get());
    assert(sequence or selector);

    if (sequence) {
        sequence->AddChild(std::move(node));
    }
    else {
        selector->AddChild(std::move(node));
    }
}