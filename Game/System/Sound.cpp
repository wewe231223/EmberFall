#include "pch.h"
#include "Sound.h"
#include "../Utility/Crash.h"
#include "../Utility/RandomEngine.h"
#include "../Game/System/Timer.h"
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


    for (auto& sound : mSoundList) {
        if (sound) {
            sound->Update(Time.GetDeltaTime<float>());
        }
    }

}

void SoundManager::PlaySound(const std::string& name, std::chrono::milliseconds delay, float volume, USHORT id, bool loop) {
    DelayedSound ds{ name, std::chrono::steady_clock::now() + delay, volume, loop };
    mDelayedSounds.emplace(std::move(ds));
}

Sound* SoundManager::PlaySound(const std::string& name, float volume, SoundOption option, std::chrono::milliseconds fadeTime) {
    auto pIt = mSounds.find(name); 

	if (pIt != mSounds.end()) {
		FMOD::Sound* sound = pIt->second;

		FMOD::Channel* channel = nullptr;
		FMOD_RESULT res = mSystem->playSound(sound, nullptr, false, &channel);
		LogFMODError(res, name.c_str());

		return mSoundList.emplace_back(std::make_unique<Internal::StandardSound>(channel, sound, volume, option, fadeTime)).get();
	}

	auto plIt = mPlayLists.find(name);
   
	if (plIt != mPlayLists.end()) {
		auto& list = plIt->second;

		if (list.empty()) {
			OutputDebugStringA("[FMOD ERROR] PlayList is empty.\n");
			return nullptr;
		}

		FMOD::Channel* channel = nullptr;
		FMOD_RESULT result = mSystem->playSound(list[0], nullptr, false, &channel);
		LogFMODError(result, ("PlayList: " + name).c_str());

		return mSoundList.emplace_back(std::make_unique<Internal::PlayListSound>(channel, list, volume, option, fadeTime)).get();
	}

    return nullptr;
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

void SoundManager::Terminate() {
    for (auto& [name, sound] : mSounds) {
        if (sound) {
            sound->release();
        }
    }

    mSounds.clear();
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

        std::vector<std::string> soundNames;
        std::string soundName;
        while (iss >> soundName) {
            soundNames.push_back(soundName);
        }

        AddPlayList(listName, soundNames);
    }
}

void SoundManager::AddPlayList(const std::string& name, const std::vector<std::string>& soundNames) {
	auto& list = mPlayLists[name];

	for (const auto& soundName : soundNames) {
		auto it = mSounds.find(soundName);
		if (it != mSounds.end()) {
			list.emplace_back(it->second);
		}
	}
}
