#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CircularBuffer.h
// 2025.01.05 김성준   - 환형 큐 처럼 동작하면서, 
//                      버퍼가 모두 찼을 때 가장 오래된 데이터를 덮어쓰는 방식을 가진 새로운 버퍼 정의
// 2025.01.06 김승범   - 원하는 기능을 추가하기에 애로사항이 있어 새롭게 작성함.  
// 2025.01.07 김승범   - CircularBuffer 로 이름을 변경하고, Emplace_back 을 추가함. 
//                      
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iterator>
#include <vector>
#include "../Utility/Crash.h"

template<typename Container>
concept RandomAccessible = requires(Container c, size_t i) {
    { c[i] } -> std::convertible_to<typename Container::value_type&>;
};

template<typename Container>
class CircularBufferIterator {
public:
	using base_iterator = typename Container::iterator;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename Container::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    CircularBufferIterator(base_iterator begin, size_t bufferSize, size_t index)
        : mBufferBegin(begin), mCurrent(begin), mBufferCapacity(bufferSize), mCurrentIndex(index) {
		std::advance(mCurrent, mCurrentIndex % mBufferCapacity);
    }

public:
    reference operator*() const {
		return *mCurrent;
    }

    CircularBufferIterator& operator++() {
        mCurrentIndex++;
		mCurrent = mBufferBegin;
        std::advance(mCurrent, mCurrentIndex % mBufferCapacity);
        return *this;
    }

    CircularBufferIterator operator++(int) {
        CircularBufferIterator temp = *this;
        ++(*this);
        return temp;
    }

    CircularBufferIterator& operator--() {
        mCurrentIndex--;
        mCurrent = mBufferBegin;
        std::advance(mCurrent, mCurrentIndex % mBufferCapacity);
        return *this;
    }

    CircularBufferIterator operator--(int) {
        CircularBufferIterator temp = *this;
        --(*this);
        return temp;
    }

    CircularBufferIterator operator+(difference_type n) const {
        return CircularBufferIterator(mBufferBegin, mBufferCapacity, mCurrentIndex + n);
    }

    CircularBufferIterator operator-(difference_type n) const {
        return CircularBufferIterator(mBufferBegin, mBufferCapacity, mCurrentIndex - n);
    }

    difference_type operator-(const CircularBufferIterator& other) const {
        return mCurrentIndex - other.mCurrentIndex;
    }

    bool operator==(const CircularBufferIterator& other) const {
        return mCurrentIndex == other.mCurrentIndex;
    }

    bool operator!=(const CircularBufferIterator& other) const {
        return mCurrentIndex != other.mCurrentIndex;
    }

    bool operator<(const CircularBufferIterator& other) const {
        return mCurrentIndex < other.mCurrentIndex;
    }

    bool operator>(const CircularBufferIterator& other) const {
        return mCurrentIndex > other.mCurrentIndex;
    }

    bool operator<=(const CircularBufferIterator& other) const {
        return mCurrentIndex <= other.mCurrentIndex;
    }

    bool operator>=(const CircularBufferIterator& other) const {
        return mCurrentIndex >= other.mCurrentIndex;
    }
private:
    const base_iterator mBufferBegin{};
	base_iterator mCurrent{};
    size_t mBufferCapacity{};
    size_t mCurrentIndex{};

};

template<typename T, size_t Capacity, template<typename, typename> class Container = std::vector, typename Allocator = std::allocator<T>>
class CircularBuffer {
public:
    using value_type = T;
    using iterator = CircularBufferIterator<Container<T, Allocator>>;
    using value_type = T;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference = const T&;
    using size_type = decltype(Capacity);

public:
    explicit CircularBuffer() : mBuffer(Capacity) {
        CrashExp(Capacity > 0, "Capacity must be greater than 0");
    }

public:
    reference operator[](size_t index) {
        CrashExp(index < mCount, "Index out of range");
        size_t actualIndex = (mHead + index) % mBuffer.size();
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[actualIndex];
        }
        else {
            auto it = std::next(mBuffer.begin(), actualIndex);
            return *it;
        }
    }

    const_reference operator[](size_t index) const {
        CrashExp(index < mCount, "Index out of range");
        size_t actualIndex = (mHead + index) % mBuffer.size();
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[actualIndex];
        }
        else {
            auto it = std::next(mBuffer.begin(), actualIndex);
            return *it;
        }
    }

public:
    void PushBack(const T& value) {
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            mBuffer[mTail] = value;
        }
        else {
            auto it = std::next(mBuffer.begin(), mTail);
            *it = value;
        }

        mTail = (mTail + 1) % mBuffer.size();
        if (mCount < mBuffer.size()) {
            ++mCount;
        }
        else {
            mHead = (mHead + 1) % mBuffer.size();
        }
    }

    template<typename... Args>
    [[maybe_unused]]
    reference EmplaceBack(Args&&... args) {
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            mBuffer[mTail] = T(std::forward<Args>(args)...);
            auto& ref = mBuffer[mTail];
            mTail = (mTail + 1) % mBuffer.size();
            if (mCount < mBuffer.size()) {
                ++mCount;
            }
            else {
                mHead = (mHead + 1) % mBuffer.size();
            }
            return ref;
        }
        else {
            auto it = std::next(mBuffer.begin(), mTail);
            *it = T(std::forward<Args>(args)...);
            auto& ref = *it;
            mTail = (mTail + 1) % mBuffer.size();
            if (mCount < mBuffer.size()) {
                ++mCount;
            }
            else {
                mHead = (mHead + 1) % mBuffer.size();
            }
            return ref;
        }
    }


    reference Front() {
        CrashExp(!Empty(), "Buffer is empty");
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[mHead];
        }
        else {
            auto it = std::next(mBuffer.begin(), mHead);
            return *it;
        }
    }

    const_reference Front() const {
        CrashExp(!Empty(), "Buffer is empty");
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[mHead];
        }
        else {
            auto it = std::next(mBuffer.begin(), mHead);
            return *it;
        }
    }

    reference Back() {
        CrashExp(!Empty(), "Buffer is empty");
        size_t backIndex = (mTail + mBuffer.size() - 1) % mBuffer.size();
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[backIndex];
        }
        else {
            auto it = std::next(mBuffer.begin(), backIndex);
            return *it;
        }
    }

    const_reference Back() const {
        CrashExp(!Empty(), "Buffer is empty");
        size_t backIndex = (mTail + mBuffer.size() - 1) % mBuffer.size();
        if constexpr (RandomAccessible<Container<T, Allocator>>) {
            return mBuffer[backIndex];
        }
        else {
            auto it = std::next(mBuffer.begin(), backIndex);
            return *it;
        }
    }

    bool Empty() const {
        return mCount == 0;
    }

    bool Full() const {
        return mCount == mBuffer.size();
    }

    size_type Size() const {
        return mCount;
    }

    iterator begin() {
        return iterator(mBuffer.begin(), mBuffer.size(), mHead);
    }

    iterator end() {
        return iterator(mBuffer.begin(), mBuffer.size(), mHead + mCount);
    }

private:
    Container<T, Allocator> mBuffer{};
    size_type mTail{};
    size_type mHead{};
    size_type mCount{};
};