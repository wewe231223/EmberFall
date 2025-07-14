#include "pch.h"
#include "Sound.h"
#include "../Utility/Crash.h"
#include "../Utility/RandomEngine.h"
#include <Windows.h>

void LogFMODError(FMOD_RESULT result, const char* context) {
    if (result != FMOD_OK) {
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

    UpdatePlayLists();
}

void SoundManager::PlaySound(const std::string& name, float volume, bool loop) {
    auto it = mSounds.find(name);
    if (it != mSounds.end()) {
        FMOD::Channel* channel = nullptr;
        FMOD_RESULT result = mSystem->playSound(it->second, nullptr, false, &channel);
        LogFMODError(result, name.c_str());

        if (result == FMOD_OK && channel) {
            channel->setVolume(volume);
            channel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
            mChannels[name] = channel;
        }
    }
}

void SoundManager::PlaySoundList(const std::string& listName, float volumeRate) {
    auto pit = mPlayLists.find(listName);
    if (pit != mPlayLists.end()) {
        PlayListData& list = pit->second;
        list.playing = true;
        list.currentIndex = 0;


        if (list.mode == PlayMode::Shuffle) {
            std::shuffle(list.sounds.begin(), list.sounds.end(), RandomEngine::GetEngine());
        }

        FMOD::Channel* channel = nullptr;
        FMOD_RESULT result = mSystem->playSound(list.sounds[0], nullptr, false, &channel);
        LogFMODError(result, ("PlayList: " + listName).c_str());

        if (result == FMOD_OK && channel) {
            channel->setVolume(list.volume * volumeRate);
            list.currentChannel = channel;
        }
    }
}

void SoundManager::StopSound(const std::string& name) {
    auto it = mChannels.find(name);
    if (it != mChannels.end()) {
        if (it->second) {
            it->second->stop();
        }
        it->second = nullptr;
    }

    auto pit = mPlayLists.find(name);
    if (pit != mPlayLists.end()) {
        PlayListData& list = pit->second;
        list.playing = false;

        if (list.currentChannel) {
            list.currentChannel->stop();
            list.currentChannel = nullptr;
        }
    }
}

void SoundManager::SetVolume(const std::string& name, float volume) {
    auto it = mChannels.find(name);
    if (it != mChannels.end()) {
        if (it->second) {
            it->second->setVolume(volume);
        }
    }

    auto pit = mPlayLists.find(name);
    if (pit != mPlayLists.end()) {
        if (pit->second.currentChannel) {
            pit->second.currentChannel->setVolume(volume);
            pit->second.volume = volume;
        }
    }
}

void SoundManager::SetMasterVolume(float volume) {
    FMOD::ChannelGroup* masterGroup = nullptr;
    FMOD_RESULT result = mSystem->getMasterChannelGroup(&masterGroup);
    LogFMODError(result, "getMasterChannelGroup");

    if (result == FMOD_OK) {
        if (masterGroup) {
            masterGroup->setVolume(volume);
        }
    }
}

void SoundManager::AddPlayList(const std::string& listName, const std::vector<std::string>& soundNames, PlayMode mode, float volume) {
    PlayListData data;
    data.mode = mode;
    data.volume = volume;

    for (const auto& name : soundNames) {
        auto it = mSounds.find(name);
        if (it != mSounds.end()) {
            data.sounds.push_back(it->second);
        }
    }

    if (!data.sounds.empty()) {
        mPlayLists[listName] = std::move(data);
    }
}

void SoundManager::UpdatePlayLists() {
    for (auto& [name, list] : mPlayLists) {
        if (!list.playing || list.sounds.empty()) {
            continue;
        }

        bool isPlaying = false;
        if (list.currentChannel) {
            list.currentChannel->isPlaying(&isPlaying);
        }

        if (!isPlaying) {
            list.currentIndex++;

            if (list.currentIndex >= list.sounds.size()) {
                list.currentIndex = 0;

                if (list.mode == PlayMode::Shuffle) {
                    std::shuffle(list.sounds.begin(), list.sounds.end(), RandomEngine::GetEngine());
                }
            }

            FMOD::Channel* nextChannel = nullptr;
            FMOD_RESULT result = mSystem->playSound(list.sounds[list.currentIndex], nullptr, false, &nextChannel);
            LogFMODError(result, ("PlayList Next: " + name).c_str());

            if (result == FMOD_OK && nextChannel) {
                nextChannel->setVolume(list.volume);
                list.currentChannel = nextChannel;
            }
        }
    }
}

void SoundManager::Terminate() {
    for (auto& [name, sound] : mSounds) {
        if (sound) {
            sound->release();
        }
    }

    mSounds.clear();
    mChannels.clear();
    mPlayLists.clear();

    if (mSystem) {
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
}

void SoundManager::LoadSoundListFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        OutputDebugStringA("[FMOD ERROR] Failed to open sound list file.\n");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name, path;
        if (!(iss >> name >> path)) {
            continue;
        }

        if (mSounds.contains(name)) {
            continue;
        }

        FMOD::Sound* sound = nullptr;
        FMOD_MODE mode = FMOD_DEFAULT | FMOD_LOOP_OFF;

        FMOD_RESULT result = mSystem->createSound(path.c_str(), mode, nullptr, &sound);
        LogFMODError(result, path.c_str());
        if (result == FMOD_OK) {
            mSounds[name] = sound;
        }
    }
}

void SoundManager::LoadPlayListFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        OutputDebugStringA("[FMOD ERROR] Failed to open playlist file.\n");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string listName, modeStr;
        float volume;
        if (!(iss >> listName >> modeStr >> volume)) {
            continue;
        }

        PlayMode mode = (modeStr == "Shuffle") ? PlayMode::Shuffle : PlayMode::Sequential;

        std::vector<std::string> soundNames;
        std::string soundName;
        while (iss >> soundName) {
            soundNames.push_back(soundName);
        }

        AddPlayList(listName, soundNames, mode, volume);
    }
}
