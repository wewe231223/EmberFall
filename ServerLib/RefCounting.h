#pragma once

class RefCounting {
public:
    RefCounting();
    ~RefCounting();

public:
    size_t AddRef();
    size_t ReleaseRef();

private:
    std::atomic_size_t mRef{ };
};
