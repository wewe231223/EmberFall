#pragma once

namespace Graphs {
    template <typename IndexT> requires std::is_arithmetic_v<IndexT>
    struct GraphNode {
        IndexT index;
    };

    template <typename IndexT> requires std::is_arithmetic_v<IndexT>
    struct GraphEdge {
        IndexT from;
        IndexT to;
    };

    template <typename IndexT, typename WeightT = double> requires std::is_arithmetic_v<IndexT>
    struct WeightedGraphEdge {
        IndexT from;
        IndexT to;
        WeightT weight;
    };

    struct PathEdge {
        SimpleMath::Vector2 source;
        SimpleMath::Vector2 dest;
        float dist;
    };

    class Graph {
    public:
        using IndexType = INT32;

        using Node = GraphNode<IndexType>;
        using Edge = GraphEdge<IndexType>;

        using NodeVector = std::vector<Node>;
        using EdgeList = std::list<Edge>;
        using AdjacencyList = std::vector<EdgeList>;

    public:
        EdgeList::iterator EdgeListBegin(IndexType idx);
        EdgeList::iterator EdgeListEnd(IndexType idx);

        size_t GetNodeSize() const;
        size_t GetEdgeSize() const;

        Node GetNode(IndexType node) const;
        Node& GetNode(IndexType node);

        Edge GetEdge(IndexType from, IndexType to) const;
        Edge& GetEdge(IndexType from, IndexType to);

        void AddNode(Node node);
        void RemoveNode(IndexType node);

        void AddEdge(Edge edge);
        void RemoveEdge(IndexType from, IndexType to);
            
    private:
        NodeVector mNodes;
        AdjacencyList mAdjacencyList; // 인접 리스트
    };

    class GraphMap {
    public:
        GameUnits::GameUnit<GameUnits::Meter> DEFAULT_CELL_SIZE = 0.5f;

    public:
        GraphMap(const std::shared_ptr<class Terrain> terrain);

        size_t ConvertPositionToIndex(const SimpleMath::Vector3& pos);

    private:
        Graph mGraph;
    };
}