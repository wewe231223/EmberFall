#include "pch.h"
#include "BuffSystem.h"

BuffSystem::BuffSystem(std::shared_ptr<GameObject> obj) 
    : mOwner{ obj } { }

BuffSystem::~BuffSystem() { }

void BuffSystem::Update(const float deltaTime) {
    for (auto& script : mBuffs) {
        if (nullptr == script or not script->IsActive()) {
            continue;
        }

        script->Update(deltaTime);
    }
}

void BuffSystem::LateUpdate(const float deltaTime) {
    for (auto& script : mBuffs) {
        if (nullptr == script) {
            continue;
        }

        if (not script->IsActive()) {
            script.reset();
            continue;
        }

        script->LateUpdate(deltaTime);
    }

    std::erase_if(mBuffs, [](const auto& script) { return nullptr == script; });
}
