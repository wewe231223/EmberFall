#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BehaviorTree.h
//  2024 - 02 - 25 : BehaviorTree 구현
//                   BehaviorTree 구현은 BT namespace 안에다가만 하자.
// 
// Sequence node : 자식 노드들을 순차적으로 실행 (SUCCESS를 반환하면 다음 자식 노드를 실행)
// Selector node : 어느 하나의 자식노드라도 성공하면 SUCCESS를 반환. (FAIL을 반환하면 다음 자식 노드를 실행)
// Leaf node : Condition(조건만을 제시, 절대로 RUNNING Status는 가지지 않음), Action (특정 행동 수행)
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

namespace BT {
    enum class NodeStatus {
        FAIL,
        SUCCESS,
        RUNNING,
    }; 

    using BTUpdateFn = std::function<NodeStatus(const float)>;
    using HandlingEventFn = std::function<void(GameEvent*)>;

    class Node abstract {
    public:
        Node() = default;
        virtual ~Node() = default;

    public:
        virtual void Start() abstract;
        virtual NodeStatus Update(const float deltaTime) abstract;
        virtual void DispatchGameEvent(GameEvent* event) abstract;
    };

    using NodePtr = std::unique_ptr<Node>;

    class SequenceNode : public Node {
    public:
        SequenceNode();
        virtual ~SequenceNode();

    public:
        virtual void Start() override;
        virtual NodeStatus Update(const float deltaTime) override;
        virtual void DispatchGameEvent(GameEvent* event) override;

        template <typename NodeType, typename... Args>
            requires std::derived_from<NodeType, Node> and std::is_constructible_v<NodeType, Args...>
        void AddChild(Args&&... args) {
            mChildren.emplace_back(std::make_unique<NodeType>(args...));
        }

        void AddChild(NodePtr&& node) {
            mChildren.emplace_back(std::move(node));
        }

    private:
        std::vector<NodePtr> mChildren;
        size_t mCurrentNode{ 0 };
    };

    class SelectorNode : public Node {
    public:
        SelectorNode();
        virtual ~SelectorNode();

    public:
        virtual void Start() override;
        virtual NodeStatus Update(const float deltaTime) override;
        virtual void DispatchGameEvent(GameEvent* event) override;

        template <typename NodeType, typename... Args> 
            requires std::derived_from<NodeType, Node> and std::is_constructible_v<NodeType, Args...>
        void AddChild(Args&&... args) {
            mChildren.emplace_back(std::make_unique<NodeType>(args...));
        }

        void AddChild(NodePtr&& node) {
            mChildren.emplace_back(std::move(node));
        }

    private:
        std::vector<NodePtr> mChildren;
        size_t mCurrentNode{ 0 };
    };

    class ConditionNode : public Node {
    public:
        explicit ConditionNode(BTUpdateFn&& fn);
        virtual ~ConditionNode();

    public:
        virtual void Start() override;
        virtual NodeStatus Update(const float deltaTime) override;
        virtual void DispatchGameEvent(GameEvent* event) override;

    private:
        BTUpdateFn mConditionFn;
    };

    class ActionNode : public Node {
    public:
        explicit ActionNode(BTUpdateFn&& fn);
        virtual ~ActionNode();

    public:
        virtual void Start() override;
        virtual NodeStatus Update(const float deltaTime) override;
        virtual void DispatchGameEvent(GameEvent* event) override;

    private:
        BTUpdateFn mActionFn;
    };

    class BehaviorTree abstract {
    public:
        virtual void Build(const std::shared_ptr<Script>& ownerScript) abstract;

        void Update(float deltaTime);

    protected:
        void SetRoot(NodePtr&& rootNode);

    private:
        NodePtr mRoot{ nullptr };
    };
}