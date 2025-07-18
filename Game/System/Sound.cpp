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

    while (!mDelayedSounds.empty() && mDelayedSounds.top().scheduledTime <= std::chrono::high_resolution_clock::now()) {
        const DelayedSound& ds = mDelayedSounds.top();

        ds.sound->Play(); 

        mDelayedSounds.pop();
    }

    std::erase_if(mSoundList, [](const std::unique_ptr<Sound>& sound) {
        return sound->Expired();
        }
    );

    for (auto& sound : mSoundList) {
        if (sound) {
            sound->Update(Time.GetDeltaTime<float>());
        }
    }


}

Sound* SoundManager::PlaySound(const std::string& name, float volume, SoundOption option, std::chrono::milliseconds fadeTime, std::chrono::milliseconds delay) {
    auto pIt = mSounds.find(name); 

    if (delay > std::chrono::milliseconds::zero()) {
        FMOD::Sound* sound = pIt->second;

        FMOD::Channel* channel = nullptr;
        FMOD_RESULT res = mSystem->playSound(sound, nullptr, true, &channel);
        LogFMODError(res, name.c_str());

        channel->setVolume(volume); 

        auto ptr =  mSoundList.emplace_back(std::make_unique<Internal::StandardSound>(channel, volume, option, fadeTime)).get();

		mDelayedSounds.push({ ptr, std::chrono::high_resolution_clock::now() + delay });

        return ptr; 
    }

	if (pIt != mSounds.end()) {
		FMOD::Sound* sound = pIt->second;

		FMOD::Channel* channel = nullptr;
		FMOD_RESULT res = mSystem->playSound(sound, nullptr, false, &channel);
		LogFMODError(res, name.c_str());

        channel->setVolume(volume);

		return mSoundList.emplace_back(std::make_unique<Internal::StandardSound>(channel, volume, option, fadeTime)).get();
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

        channel->setVolume(volume); 

		return mSoundList.emplace_back(std::make_unique<Internal::PlayListSound>(mSystem, list, channel, volume, option, fadeTime)).get();
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

void SoundManager::Reset() {
	for (auto& sound : mSoundList) {
		if (sound) {
            sound->Stop(); 
		}
	}

	mSoundList.clear();
	while (!mDelayedSounds.empty()) {
		mDelayedSounds.pop();
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

Internal::StandardSound::StandardSound(FMOD::Channel* channel) {
	mChannel = channel;
}

Internal::StandardSound::StandardSound(FMOD::Channel* channel, float volume, SoundOption option) {
	mChannel = channel;
	mVolume = volume;
	mOption = option;
}

Internal::StandardSound::StandardSound(FMOD::Channel* channel, float volume, SoundOption option, std::chrono::milliseconds fadeTime) {
	mChannel = channel;
	mVolume = volume;
	mOption = option;
	mFadeTime = fadeTime;
	if (mFadeTime > std::chrono::milliseconds::zero()) {
        mFadeVariable = 0.f; 
	}
}

Internal::StandardSound::~StandardSound() {
}

void Internal::StandardSound::Update(float deltaTime) {
	if (mExpired) {
		return;
	}

    if (mFadeTime > std::chrono::milliseconds::zero()) {
        float fadeDuration = static_cast<float>(mFadeTime.count()) * 0.001f;
        float fadeSpeed = 1.0f / fadeDuration;

        mFadeVariable += fadeSpeed * deltaTime;
        mFadeVariable = std::clamp(mFadeVariable, 0.0f, 1.0f);
    }

	mChannel->setVolume(mVolume * mFadeVariable);


    bool isPlaying{};
	mChannel->isPlaying(&isPlaying);

	if (mPlayState and not isPlaying) {
		mExpired = true;
	}
}

void Internal::StandardSound::SetVolume(float volume) {
	mVolume = volume;
}

void Internal::StandardSound::Pause() {
    mPlayState = false;

	FMOD_RESULT result = mChannel->setPaused(true);
	LogFMODError(result, "StandardSound::Pause");
}

void Internal::StandardSound::Play() {
	mPlayState = true;

	FMOD_RESULT result = mChannel->setPaused(false);
	LogFMODError(result, "StandardSound::Play");
}

void Internal::StandardSound::Stop() {
    FMOD_RESULT result = mChannel->setPaused(true);
    LogFMODError(result, "StandardSound::Stop");

    mExpired = true; 
}

void Internal::StandardSound::SetOption(SoundOption option) {
	mOption = option;
}

void Internal::StandardSound::Terminate() {

}

bool Internal::StandardSound::Expired() const {
    return mExpired;
}

Internal::PlayListSound::PlayListSound(FMOD::System* system, std::vector<FMOD::Sound*> sounds, FMOD::Channel* channel, float volume, SoundOption option) : mSounds(sounds) {
	mSystem = system;
	mCurrentChannel = channel;
	mVolume = volume;
	mOption = option;
}

Internal::PlayListSound::PlayListSound(FMOD::System* system, std::vector<FMOD::Sound*> sounds, FMOD::Channel* channel, float volume, SoundOption option, std::chrono::milliseconds fadeTime) : mSounds(sounds) {
	mSystem = system;
	mCurrentChannel = channel;
	mVolume = volume;
	mOption = option;
	mFadeTime = fadeTime;
	if (mFadeTime > std::chrono::milliseconds::zero()) {
		mFadeVariable = 0.f;
	}
}

Internal::PlayListSound::~PlayListSound() {

}

void Internal::PlayListSound::Update(float deltaTime) {
	if (mExpired) {
		return;
	}

	if (mFadeTime > std::chrono::milliseconds::zero()) {
		float fadeDuration = static_cast<float>(mFadeTime.count()) * 0.001f;
		float fadeSpeed = 1.0f / fadeDuration;

		mFadeVariable += fadeSpeed * deltaTime;
		mFadeVariable = std::clamp(mFadeVariable, 0.0f, 1.0f);
	}

    if (mFadeTime > std::chrono::milliseconds::zero()) {
        mCurrentChannel->setVolume(mVolume * mFadeVariable);
    }
	bool isPlaying{};
	mCurrentChannel->isPlaying(&isPlaying);
    
	if (isPlaying) {
		return; 
	}

    if (mOption & SoundOption::Shuffle) {
		std::shuffle(mSounds.begin(), mSounds.end(), RandomEngine::GetEngine());
    }

	mCurrentIndex = (mCurrentIndex + 1) % mSounds.size();

	FMOD::Sound* nextSound = mSounds[mCurrentIndex];
	FMOD_RESULT result = mSystem->playSound(nextSound, nullptr, false, &mCurrentChannel);
	LogFMODError(result, "PlayListSound::Update");

	if (result == FMOD_OK) {

		if (mFadeTime > std::chrono::milliseconds::zero()) {
            mCurrentChannel->setVolume(mVolume * mFadeVariable);
        }
        else {
		    mCurrentChannel->setVolume(mVolume);
        }

		mPlayState = true;
	}
	

}

void Internal::PlayListSound::SetVolume(float volume) {
    mVolume = volume;
}

void Internal::PlayListSound::Pause() {
	mPlayState = false;

	FMOD_RESULT result = mCurrentChannel->setPaused(true);
	LogFMODError(result, "PlayListSound::Pause");
}

void Internal::PlayListSound::Play() {
	mPlayState = true;

	FMOD_RESULT result = mCurrentChannel->setPaused(false);
	LogFMODError(result, "PlayListSound::Play");
}

void Internal::PlayListSound::Stop() {
	FMOD_RESULT result = mCurrentChannel->setPaused(true);
	LogFMODError(result, "PlayListSound::Stop");

	mExpired = true;
	mCurrentIndex = 0;
	mCurrentChannel = nullptr;
}

void Internal::PlayListSound::SetOption(SoundOption option) {
	mOption = option;
}

void Internal::PlayListSound::Terminate() {
}

bool Internal::PlayListSound::Expired() const {
    return mExpired;
}
