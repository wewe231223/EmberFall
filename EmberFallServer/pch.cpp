#include "pch.h"
#include "CollisionWorld.h"

std::unique_ptr<CollisionWorld> gCollisionWorld = std::make_unique<CollisionWorld>();