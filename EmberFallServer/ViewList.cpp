#include "pch.h"
#include "ViewList.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "GameTimer.h"

ViewList::ViewList() { }

ViewList::~ViewList() { }

void ViewList::Update() {

}

std::unordered_set<NetworkObjectIdType> ViewList::GetCurrViewList() const {
    return mViewList;
}
