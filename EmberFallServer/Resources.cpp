#include "pch.h"
#include "Resources.h"

void ResourceManager::LoadEnvFromFile(const std::filesystem::path& path) {
    std::ifstream envs{ path, std::ios::binary };
    if (not envs.is_open()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Load Environments Failure - File does not exists");
        return;
    }

    uint32_t numOfEnvs{ };
    envs.read(reinterpret_cast<char*>(&numOfEnvs), sizeof(numOfEnvs));
    
    std::vector<EnvironmentsInfo> infos(numOfEnvs);
    envs.read(reinterpret_cast<char*>(infos.data()), infos.size() * sizeof(EnvironmentsInfo));

    for (auto& info : infos) {
        mEnvInfos.try_emplace(info.envType, info);
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Env File Load Sucess");
}

void ResourceManager::LoadAnimationFromFile(const std::filesystem::path& path) {
    std::ifstream envs{ path, std::ios::binary };
    if (not envs.is_open()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Load Animations Failure");
        return;
    }

    uint32_t numOfKeys{ };
    envs.read(reinterpret_cast<char*>(&numOfKeys), sizeof(numOfKeys));

    std::string key{ };
    uint32_t keyLength{ };
    uint32_t numOfDurations{ };
    for (uint32_t i = 0; i < numOfKeys; ++i) {
        envs.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
        key.resize(keyLength);

        envs.read(key.data(), keyLength);
        mAnimInfos.try_emplace(key, AnimationInfos{ });

        envs.read(reinterpret_cast<char*>(&numOfDurations), sizeof(numOfDurations));
        if (NUM_OF_ANIM != numOfDurations) {
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "File Format Is Incorrect");
        }
        envs.read(reinterpret_cast<char*>(mAnimInfos[key].durations), numOfDurations * sizeof(float));
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Anim File Load Sucess");
}

void ResourceManager::LoadEntityFromFile(const std::filesystem::path& path) {
    // file format
    //  [UINT] // num of infos
    // 
    //      // information format
    //      [UINT] [char[size]]     name
    //      [BoundingOrientedBox]   bb
    //      [SimpleMath::Vector3]   pivot

    std::ifstream envs{ path, std::ios::binary };
    if (not envs.is_open()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Load Entities Failure");
        return;
    }

    uint32_t numOfEntities{ };
    envs.read(reinterpret_cast<char*>(&numOfEntities), sizeof(numOfEntities));

    std::string name{ };
    uint32_t nameLen{ };
    for (uint32_t i = 0; i < numOfEntities; ++i) {
        envs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        name.resize(nameLen);

        envs.read(name.data(), nameLen);
        mEntityInfos.try_emplace(name, EntityInfo{ });

        decltype(auto) info = mEntityInfos[name];
        envs.read(reinterpret_cast<char*>(&info), sizeof(DirectX::BoundingBox) + sizeof(SimpleMath::Vector3));
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Anim File Load Sucess");
}

const EntityInfo& ResourceManager::GetEntityInfo(const std::string& name) {
    if (not mEntityInfos.contains(name)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Access - Invalid Entity Name: {}", name);
        Crash("");
    }

    return mEntityInfos[name];
}

const AnimationInfos& ResourceManager::GetAnimInfo(const std::string& key) {
    if (not mAnimInfos.contains(key)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Access - Invalid Animation Key: {}", key);
        Crash("");
    }

    return mAnimInfos[key];
}

const EnvironmentsInfo& ResourceManager::GetEnvInfo(GameProtocol::EnvironmentType envType) {
    if (not mEnvInfos.contains(envType)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Access - Invalid EnvType: {}", static_cast<int32_t>(envType));
        Crash("");
    }

    return mEnvInfos[envType];
}
