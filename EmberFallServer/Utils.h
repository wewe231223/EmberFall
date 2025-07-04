#pragma once

#include "GameSession.h"

template <typename T>
struct GetGameObjectTag;

template <>
struct GetGameObjectTag<GameSession> {
    using type = GameSession;
};

//
//template <>
//struct GetGameObjectTag<NPC> {
//
//};