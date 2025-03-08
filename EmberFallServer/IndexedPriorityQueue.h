#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IndexedPriorityQueue.h (header only)
// 
// 2025 - 03 - 06 : AStar, Dijikstra 구현에서 유용하게 쓰일만한 Index있는 우선순위 큐 구현
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <algorithm>
#include <vector>

template <typename IndexType, typename KeyType>
    requires std::is_integral_v<IndexType>
class IndexedPriorityQueue {
public:
    using Index = IndexType;
    using Key = KeyType;

    using IndexVector = std::vector<Index>;
    using KeyVector = std::vector<Key>;

public:
    IndexedPriorityQueue() = default;
    ~IndexedPriorityQueue() = default;

    // 초기화
    IndexedPriorityQueue(size_t size, Index val = Index{ }, Key key = Key{ }) {
        mKeys.resize(size, key);
        mHeap.reserve(size);
    }

public:
    bool IsHeap() const {
        return std::is_heap(mHeap.begin(), mHeap.end(), mCompFn);
    }

    size_t Size() const {
        return mSize;
    }

    Key GetKeyVal(Index idx) const {
        return mKeys[idx];
    }

    bool Empty() const {
        return 0 == mSize;
    }

    void SwapVal(Index idx1, Index idx2) {
        std::swap(mHeap[idx1], mHeap[idx2]);
    }

    void MakeHeap() {
        std::make_heap(mHeap.begin(), mHeap.end(), mCompFn);
    }

    void Insert(Index val, Key key) {
        ++mSize;

        mKeys[val] = key;
        mHeap.push_back(val);
        std::push_heap(mHeap.begin(), mHeap.end(), mCompFn);
    }

    Index Pop() {
        --mSize;

        std::pop_heap(mHeap.begin(), mHeap.end(), mCompFn);
        auto rtVal = mHeap.back();
        mHeap.pop_back();
        return rtVal;
    }

    void ChangeKeyVal(Index val, Key key) {
        mKeys[val] = key;

        MakeHeap();
    }

private:
    bool CompareLess(const Index& idx1, const Index& idx2) {
        return mKeys[idx1] < mKeys[idx2];
    }

    bool CompareGreater(const Index& idx1, const Index& idx2) {
        return mKeys[idx1] > mKeys[idx2];
    }

private:
    KeyVector mKeys{ };
    IndexVector mHeap{ };

    size_t mSize{ 0ULL };

    std::function<bool(const Index&, const Index&)> mCompFn{ std::bind_front(&IndexedPriorityQueue::CompareGreater, this) };
};