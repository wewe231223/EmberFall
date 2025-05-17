#include "pch.h"
#include "RefCounting.h"

RefCounting::RefCounting() 
    : mRef{ 1 } { }

RefCounting::~RefCounting() { }

size_t RefCounting::AddRef() {
    return mRef.fetch_add(1);
}

size_t RefCounting::ReleaseRef() {
    return mRef.fetch_sub(1);
}
