#pragma once 
#include "../External/Include/Fmod/fmod.hpp"
#include "../External/Include/Fmod/fmod_errors.h"

#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/fmod_vc.lib")
#else
#pragma comment(lib, "External/lib/release/fmod_vc.lib")
#endif

#include <string>
#include <unordered_map>
#include <chrono>
#include <queue>
#include <vector>
#include <random>

#ifdef PlaySound
#undef PlaySound
#endif

// 1. 사운드 꺼질때 fade out 

// 추가할 것들 
// 1. 다른 플레이어 발소리 - 범위 따라서 볼륨 조절 
// 2. 보스 몬스터 / 잡 몬스터 걷는 소리 ( 인간이랑 같은걸로 ) - 범위 따라서 볼륨 조절 
// 3. 보스 몬스터 / 잡 몬스터 공격 소리 
// 4. 보스 몬스터 / 잡 몬스터 피격 소리 
// 5. 보석 안개 소리 
// 6. 플레이어 사망 소리 


class SoundManager {
public:
    enum class PlayMode { Sequential, Shuffle };

private:
    struct PlayListData {
        std::vector<FMOD::Sound*> sounds{};
        PlayMode mode{};
        size_t currentIndex{ 0 };
        bool playing{ false };
        float volume{ 1.f };
        FMOD::Channel* currentChannel{};
    };

    struct DelayedSound {
        std::string name{};
        std::chrono::steady_clock::time_point scheduledTime{};
        float volume{};
        bool loop{};

        bool operator>(const DelayedSound& other) const {
            return scheduledTime > other.scheduledTime;
        }
    };

private:
    SoundManager() = default;

public:
    static SoundManager& GetInstance();

    bool Initialize();
    void Update();
    void Terminate();

    void PlaySound(const std::string& name, float volume = 1.f, bool loop = false);
	void PlaySound(const std::string& name, std::chrono::milliseconds delay, float volume = 1.f, bool loop = false);

    void PlaySoundList(const std::string& listName, float volumeRate = 1.f);

    void StopSound(const std::string& name);
    void SetVolume(const std::string& name, float volume);
    void SetMasterVolume(float volume);

    void AddPlayList(const std::string& listName, const std::vector<std::string>& soundNames, PlayMode mode, float volume = 1.0f);
    void LoadSoundListFromFile(const std::string& filepath);
    void LoadPlayListFromFile(const std::string& filepath);

private:
    void UpdatePlayLists();

private:
    FMOD::System* mSystem{ nullptr };

    std::priority_queue<DelayedSound, std::vector<DelayedSound>, std::greater<>> mDelayedSounds{};

    std::unordered_map<std::string, FMOD::Sound*> mSounds{};
    std::unordered_map<std::string, FMOD::Channel*> mChannels{};
    std::unordered_map<std::string, PlayListData> mPlayLists{};
};
