#include <array>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Crash.h
// 2025 - 01 - 05 김성준 - 환형 큐 처럼 동작하면서, 
//        버퍼가 모두 찼을 때 가장 오래된 데이터를 덮어쓰는 방식을 가진 새로운 버퍼 정의
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, size_t size>
class InputBufferIterator {
public:
    #ifdef _HAS_CXX20
        using iterator_concept = std::random_access_iterator;
    #endif

    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using iterator_category = std::random_access_iterator_tag;

public:
    InputBufferIterator() noexcept { }

    InputBufferIterator(pointer ptr, size_t offset = 0) noexcept : mPtr{ ptr }, mIndex{ offset } { }

    ~InputBufferIterator() noexcept { }

    InputBufferIterator(const InputBufferIterator& other) noexcept {
        mPtr = other.mPtr;
        mIndex = other.mIndex;
    }

    InputBufferIterator& operator=(const InputBufferIterator& other) noexcept {
        mPtr = other.mPtr;
        mIndex = other.mIndex;
        return *this;
    }

    InputBufferIterator(InputBufferIterator&& other) noexcept {
        mPtr = other.mPtr;
        mIndex = other.mIndex;
    }

    InputBufferIterator& operator=(InputBufferIterator&& other) noexcept {
        mPtr = other.mPtr;
        mIndex = other.mIndex;
        return *this;
    }

public:
    reference operator*() {
        return *(mPtr + mIndex);
    }

    void operator++() {
        ++mIndex;
        if (mIndex >= size) {
            mIndex = 0;
        }
    }

    InputBufferIterator operator++(int) {
        InputBufferIterator<T, size> iter{ *this };
        ++mIndex;
        if (mIndex >= size) {
            mIndex = 0;
        }
        return iter;
    }

    void operator--() {
        --mIndex;
        if (mIndex < 0) {
            mIndex = size;
        }
    }

    InputBufferIterator operator--(int) {
        InputBufferIterator<T, size> iter{ *this };
        --mIndex;
        if (mIndex < 0) {
            mIndex = size;
        }
        return iter;
    }

    bool operator==(const InputBufferIterator& iter) {
        return mIndex == iter.mIndex;
    }

    bool operator!=(const InputBufferIterator& iter) {
        return mIndex != iter.mIndex;
    }

private:
    pointer mPtr{ nullptr };
    size_t mIndex{ };
};

template <typename T, size_t size>
class InputBuffer {
public:
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using size_type = size_t;

    using iterator = InputBufferIterator<T, size>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    using Container = std::array<T, size>;

public:
    InputBuffer() noexcept { }
    ~InputBuffer() noexcept { }

    InputBuffer(const InputBuffer& other) noexcept {

    }

    InputBuffer(InputBuffer&& other) noexcept {

    }

    InputBuffer& operator=(const InputBuffer& other) noexcept {
        return *this;
    }

    InputBuffer& operator=(InputBuffer&& other) noexcept {
        return *this;
    }

public:
    size_t MaxSize() const noexcept {
        return size;
    }

    size_t Size() const noexcept {
        return mTail;
    }

    iterator Push(const T& data) {
        mBuffer[mTail++] = data;
        if (mTail >= size) {
            mTail = 0;
        }

        if (mTail == mHead) {
            ++mHead;
            if (mHead >= size) {
                mHead = 0;
            }
        }

        return iterator{ mBuffer.data(), mTail };
    }

public:
    iterator begin() {
        return iterator{ mBuffer.data(), mHead };
    }

    iterator end() {
        return iterator{ mBuffer.data(), mTail };
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

private:
    Container mBuffer{ };
    size_t mHead{ };
    size_t mTail{ };
};