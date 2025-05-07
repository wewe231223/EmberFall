#pragma once

#include "BuffScript.h"

class BuffSystem {
public:
    BuffSystem(std::shared_ptr<GameObject> obj);
    ~BuffSystem();

public:
    template <typename BuffScriptT, typename ...Args>
        requires std::derived_from<BuffScriptT, BuffScript>
    void AddBuff(Args&&... args);

    void Update(const float deltaTime);
    void LateUpdate(const float deltaTime);

private:
    std::shared_ptr<GameObject> mOwner{ };
    std::vector<std::shared_ptr<BuffScript>> mBuffs{ };
};

template<typename BuffScriptT, typename ...Args>
    requires std::derived_from<BuffScriptT, BuffScript>
inline void BuffSystem::AddBuff(Args && ...args) {
    if (nullptr == mOwner) {
        return;
    }

    auto script = std::make_shared<BuffScriptT>(mOwner, args...);
    script->Init();
    script->SetActive(true);
    mBuffs.emplace_back(script);
}
