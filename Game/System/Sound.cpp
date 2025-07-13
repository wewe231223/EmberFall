#include "pch.h"
#include "Sound.h"
#include "../Utility/Crash.h"

void LogFMODError(FMOD_RESULT result, const char* context)
{
    if (result != FMOD_OK)
    {
        std::string msg = "[FMOD ERROR] ";
        msg += context;
        msg += " : ";
        msg += FMOD_ErrorString(result);
        msg += "\n";

        OutputDebugStringA(msg.c_str());
    }
}

SoundManager& SoundManager::GetInstance() {
    static SoundManager instance{};
    return instance;
}

bool SoundManager::Initialize() {
    FMOD_RESULT result = FMOD::System_Create(&mSystem);
    LogFMODError(result, "System_Create");
    CrashExp(result == FMOD_OK, "FMOD::System_Create failed");

    result = mSystem->init(512, FMOD_INIT_NORMAL, nullptr);
    LogFMODError(result, "System::init");
    CrashExp(result == FMOD_OK, "FMOD::System::init failed");

    return true;
}

void SoundManager::Update() {
    if (mSystem) {
        mSystem->update();
    }
}

void SoundManager::LoadSound(const std::string& name, const std::string& filepath, bool loop) {
    if (mSounds.contains(name)) {
        return;
    }

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;

    if (loop) {
        mode |= FMOD_LOOP_NORMAL;
    }
    else {
        mode |= FMOD_LOOP_OFF;
    }

    FMOD_RESULT result = mSystem->createSound(filepath.c_str(), mode, nullptr, &sound);
    LogFMODError(result, filepath.c_str());
    CrashExp(result == FMOD_OK, "FMOD::System::createSound failed");

    mSounds[name] = sound;
}

void SoundManager::PlaySound(const std::string& name, float volume) {
    auto it = mSounds.find(name);
    if (it == mSounds.end()) {
        return;
    }

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(it->second, nullptr, false, &channel);
    LogFMODError(result, name.c_str());

    if (result == FMOD_OK && channel) {
        channel->setVolume(volume);
        mChannels[name] = channel;
    }
}

void SoundManager::StopSound(const std::string& name) {
    auto it = mChannels.find(name);

    if (it != mChannels.end() && it->second) {
        it->second->stop();
    }
}

void SoundManager::SetVolume(const std::string& name, float volume) {
    auto it = mChannels.find(name);

    if (it != mChannels.end() && it->second) {
        it->second->setVolume(volume);
    }
}

void SoundManager::SetMasterVolume(float volume) {
    FMOD::ChannelGroup* masterGroup = nullptr;

    FMOD_RESULT result = mSystem->getMasterChannelGroup(&masterGroup);
    LogFMODError(result, "getMasterChannelGroup");

    if (result == FMOD_OK && masterGroup) {
        masterGroup->setVolume(volume);
    }
}

void SoundManager::Terminate() {
    for (auto& [name, sound] : mSounds) {
        if (sound)
            sound->release();
    }

    mSounds.clear();
    mChannels.clear();

    if (mSystem) {
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
}
