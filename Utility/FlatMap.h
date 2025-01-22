#pragma once 
#include <vector>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include "../Utility/Crash.h"

template <typename Key, typename Value, typename Compare = std::less<Key>>
class FlatMap {
public:
    using value_type = std::pair<Key, Value>;
    using container_type = std::vector<value_type>;
    using size_type = typename container_type::size_type;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

public:
    FlatMap() = default;

    FlatMap(std::initializer_list<value_type> init) {
        Insert(init);
    }

    template <typename InputIt>
    FlatMap(InputIt first, InputIt last) {
        Insert(first, last);
    }

    ~FlatMap() = default;
    FlatMap(const FlatMap&) = default;
    FlatMap& operator=(const FlatMap&) = default;

    FlatMap(FlatMap&&) noexcept = default;
    FlatMap& operator=(FlatMap&&) noexcept = default;

public:
    iterator begin() noexcept {
        return mData.begin();
    }

    const_iterator begin() const noexcept {
        return mData.begin();
    }

    iterator end() noexcept {
        return mData.end();
    }

    const_iterator end() const noexcept {
        return mData.end();
    }

    bool Empty() const noexcept {
        return mData.empty();
    }

    size_type Size() const noexcept {
        return mData.size();
    }

    size_type MaxSize() const noexcept {
        return mData.max_size();
    }

    void Clear() noexcept {
        mData.clear();
    }

    void Insert(const value_type& value) {
        InsertSorted(value);
    }

    void Insert(std::initializer_list<value_type> ilist) {
        for (const auto& value : ilist) {
            InsertSorted(value);
        }
    }

    template <typename InputIt>
    void Insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            InsertSorted(*first);
        }
    }

    void Erase(const Key& key) {
        auto it = LowerBound(key);
        if (it != mData.end() && it->first == key) {
            mData.erase(it);
        }
    }

    iterator Erase(iterator pos) {
        return mData.erase(pos);
    }

    iterator Erase(iterator first, iterator last) {
        return mData.erase(first, last);
    }

    Value& At(const Key& key) {
        auto it = LowerBound(key);
        CrashExp(it != mData.end() && it->first == key, "Key not found");
        return it->second;
    }

    const Value& At(const Key& key) const {
        const auto it = LowerBound(key);
		CrashExp(it != mData.end() && it->first == key, "Key not found");
        return it->second;
    }

    Value& operator[](const Key& key) {
        auto it = LowerBound(key);
        if (it == mData.end() or it->first != key) {
            it = mData.insert(it, std::make_pair(key, Value{}));
        }
        return it->second;
    }

    const Value& operator[](const Key& key) const {
        const auto it = LowerBound(key);
        if (it == mData.end() or it->first != key) {
            it = mData.insert(it, std::make_pair(key, Value{}));
        }
        return it->second;
    }

    iterator Find(const Key& key) {
        auto it = LowerBound(key);
        return (it != mData.end() && it->first == key) ? it : mData.end();
    }

    const_iterator Find(const Key& key) const {
        auto it = LowerBound(key);
        return (it != mData.end() && it->first == key) ? it : mData.end();
    }

    Compare Predicate() const {
        return mPredicate;
    }

    const container_type& Data() const {
        return mData;
    }

    friend bool operator==(const FlatMap& lhs, const FlatMap& rhs) {
        return lhs.mData == rhs.mData;
    }

    friend bool operator!=(const FlatMap& lhs, const FlatMap& rhs) {
        return !(lhs == rhs);
    }

private:
    decltype(auto) LowerBound(const Key& key) const {
        return std::lower_bound(mData.begin(), mData.end(), key,
            [this](const value_type& pair, const Key& key) {
                return mPredicate(pair.first, key);
            });
    }

    decltype(auto) LowerBound(const Key& key) {
        return std::lower_bound(mData.begin(), mData.end(), key,
            [this](const value_type& pair, const Key& key) {
                return mPredicate(pair.first, key);
            });
    }

    void InsertSorted(const value_type& value) {
        auto it = LowerBound(value.first);
        if (it != mData.end() && it->first == value.first) {
            it->second = value.second;
        }
        else {
            mData.insert(it, value);
        }
    }

private:
    container_type mData{};
    Compare mPredicate{};
};