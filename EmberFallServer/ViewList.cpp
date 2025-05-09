#include "pch.h"
#include "ViewList.h"
#include "GameObject.h"

ViewList::ViewList() { }

ViewList::~ViewList() { }

ViewList::ViewList(const ViewList& other) 
    : mViewRange{ other.mViewRange } {
    mViewList = other.mViewList;
}

ViewList::ViewList(ViewList&& other) noexcept
    : mViewRange{ other.mViewRange } {
    mViewList = std::move(other.mViewList);
}

ViewList& ViewList::operator=(const ViewList& other) {
    mViewRange = other.mViewRange;
    //std::copy(other.mViewList.begin(), other.mViewList.end(), mViewList.begin());
    mViewList = other.mViewList;
    return *this;
}

ViewList& ViewList::operator=(ViewList&& other) noexcept {
    mViewRange = other.mViewRange;
    mViewList = std::move(other.mViewList);
    return *this;
}

bool ViewList::IsInList(NetworkObjectIdType id) const {
    return mViewList.contains(id);
}

bool ViewList::TryInsert(NetworkObjectIdType id) {
    auto ret = mViewList.insert(id);
    return ret.second;
}

std::unordered_set<NetworkObjectIdType> ViewList::GetCurrViewList() const {
    return mViewList;
}
