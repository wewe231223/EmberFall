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
//         03 - 01 : 몬스터가 따라가는 것 까진 만들었는데... Random Location으로 이동하는 행동이 수행중이면
//                   무슨 일이 있어도(플레이어가 가까이 있더라도) 그 행동을 완수한다..
//                   매 프레임마다 트리를 탐색하긴 하나 RUNNING 상태에서 다른 행동으로 Reject되는 경우가 전혀 없다.
// 
//                   Decide Function을 두는 건 어떨까?
//                   이러면 구현상에 변경점이 생긴다.
//                   먼저 Root는 Decide Function을 수행해서 각자 Child 노드의 Weight를 계산한다.
//                   가장 Weight가 높은 노드를 선택한다. 이때 두가지 경우가 생긴다.
//                   
//                   1. 이미 Activated 인 경우 -> 계속 업데이트를 진행한다.
//                   2. Activared 노드와 다른 노드인 경우 -> 기존 활성화 노드는 Reset, 새 노드를 활성화
// 
//                   그런데 매 프레임마다 Decide Function을 모두 실행하는 건 코스트가 높다.
//                   그러면 일정 초마다 업데이트 하게 만들자.
// 
//          04 - 04 : Wating 노드 추가
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
        BehaviorTree() = default;
        virtual ~BehaviorTree() = default;

    public:
        virtual float CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const abstract;
        virtual void Build(const std::shared_ptr<Script>& ownerScript) abstract;
        virtual void DispatchGameEvent(GameEvent* event) abstract;

        void Start();
        void Update(float deltaTime);

    protected:
        void SetRoot(NodePtr&& rootNode);
        void SetOtherTree(BehaviorTree& tree);
        void SetChild(NodePtr&& node);

    private:
        NodePtr mRoot{ nullptr };
    };
}