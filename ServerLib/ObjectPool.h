#pragma once

#include <queue>

namespace PoolPointer {
    using PointerType = BYTE;

    inline constexpr PointerType Raw = 0;
    inline constexpr PointerType SharedPtr = 1;

    template <typename T, PointerType Type>
    struct Pointer {
        using Ptr = Pointer<T, Type>::Ptr;
    };

    template <typename T>
    struct Pointer<T, Raw> {
        using Ptr = T*;
    };

    template <typename T>
    struct Pointer<T, SharedPtr> {
        using Ptr = std::shared_ptr<T>;
    };
}

template <typename ObjectType, size_t size, PoolPointer::PointerType pointerType = PoolPointer::SharedPtr>
class ObjectPool {
public:
    using Pointer = PoolPointer::Pointer<ObjectType, pointerType>::Ptr;

public:
    ObjectPool() {
        for (size_t i = 0; i < size; ++i) {
            mObjectList.emplace(new ObjectType{ });
        }
    }

    ~ObjectPool() {
        if constexpr (pointerType == PoolPointer::Raw) {
            while (false == mObjectList.empty()) {
                Pointer ptr = Get();
                delete ptr;
            }
        }
    }

public:
    size_t Size() const {
        return mObjectList.size();
    }

    bool Empty() const {
        return mObjectList.empty();
    }


    Pointer Get() {
        if (mObjectList.empty()) {
            return nullptr;
        }

        Pointer ptr = mObjectList.front();
        mObjectList.pop();

        return ptr;
    }

    void Push(Pointer&& ptr) {
        mObjectList.emplace(ptr);
    }

    void Push(const Pointer& ptr) {
        mObjectList.emplace(ptr);
    }

private:
    std::queue<Pointer> mObjectList{ };
};
