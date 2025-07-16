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


enum SoundOption : unsigned int {
    NONE = 0,
	Shuffle = 1 << 0,
	Loop = 1 << 1,
};

class Sound abstract {
public:
	virtual ~Sound() = default;
    
    virtual void Update(float deltaTime) PURE; 

    virtual void SetVolume(float volume) PURE;
    virtual void Pause() PURE; 
    virtual void Play() PURE; 
    virtual void Stop() PURE;
	virtual void SetOption(SoundOption option) PURE;
	virtual void Terminate() PURE;
};


namespace Internal {
    class StandardSound : public Sound {
    public:
        StandardSound(FMOD::Channel* channel);
        StandardSound(FMOD::Channel* channel, float volume, SoundOption option);
		StandardSound(FMOD::Channel* channel, float volume, SoundOption option, std::chrono::milliseconds fadeTime);

		~StandardSound() override;

    public:
		void Update(float deltaTime) override;

		void SetVolume(float volume) override;
		void Pause() override;
		void Play() override;
		void Stop() override;
		void SetOption(SoundOption option) override;
		void Terminate() override;

	private:
		FMOD::Channel* mChannel{ nullptr };
        SoundOption mOption{ SoundOption::NONE }; 

		bool mPlayState{ false };
        float mVolume{ 1.f }; 

		std::chrono::milliseconds mFadeTime{ 0ms };
        float mFadeVariable{ -1.f };
    };

    class PlayListSound : public Sound {
    public:
        PlayListSound(const std::vector<FMOD::Sound*>& sounds, float volume, SoundOption option);
        PlayListSound(const std::vector<FMOD::Sound*>& sounds, float volume, SoundOption option, std::chrono::milliseconds fadeTime);

        ~PlayListSound() override;
    public:
		void Update(float deltaTime) override;

        void SetVolume(float volume) override;
		void Pause() override;
        void Play() override;
        void Stop() override;
        void SetOption(SoundOption option) override;
        void Terminate() override;

    private:
        const std::vector<FMOD::Sound*>& mSounds;
        SoundOption mOption{ SoundOption::NONE };

        bool mPlayState{ false };
        float mVolume{ 1.f };

        size_t mCurrentIndex{ 0 };
        FMOD::Channel* mCurrentChannel{ nullptr };

        std::chrono::milliseconds mFadeTime{ 0ms };
        float mFadeVariable{ -1.f };
    }; 
}



class SoundManager {
    struct DelayedSound {
        std::string name{};
        std::chrono::steady_clock::time_point scheduledTime{};
        float volume{};
		SoundOption option{ SoundOption::NONE };
		std::chrono::milliseconds fadeTime{ 0ms };

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
   
    void PlaySound(const std::string& name, float volume = 1.f, USHORT id, bool loop = false);
	void PlaySound(const std::string& name, std::chrono::milliseconds delay, float volume = 1.f, USHORT id = std::numeric_limits<USHORT>::max(), bool loop = false);

    void PlaySoundList(const std::string& listName, float volumeRate = 1.f, USHORT id = std::numeric_limits<USHORT>::max());

    Sound* PlaySound(const std::string& name, float volume = 1.f, SoundOption option = SoundOption::NONE, std::chrono::milliseconds fadeTime = std::chrono::milliseconds(0)); 

    void SetMasterVolume(float volume);

    void LoadSoundListFromFile(const std::string& filepath);
    void LoadPlayListFromFile(const std::string& filepath);
private:
    void AddPlayList(const std::string& name, const std::vector<std::string>& soundNames); 
private:
    FMOD::System* mSystem{ nullptr };

    std::priority_queue<DelayedSound, std::vector<DelayedSound>, std::greater<>> mDelayedSounds{};

    std::unordered_map<std::string, FMOD::Sound*> mSounds{};
    std::unordered_map<std::string, std::vector<FMOD::Sound*>> mPlayLists{};

    std::vector<std::unique_ptr<Sound>> mSoundList{};
};
