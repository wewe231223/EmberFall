#pragma once 
#include "../External/Include/Fmod/fmod.hpp"
#include "../External/Include/Fmod/fmod_errors.h"

#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/fmod_vc.lib")
#else
#pragma comment(lib, "External/lib/release/fmod_vc.lib")
#endif // DEBUG

#include <string>
#include <unordered_map>

#ifdef PlaySound
#undef PlaySound
#endif 

class SoundManager {
private:
	SoundManager() = default;

public:
    static SoundManager& GetInstance();

    bool Initialize();
    void Update(); 

    void LoadSound(const std::string& name, const std::string& filepath, bool loop = false);
    void PlaySound(const std::string& name, float volume = 1.0f);
    void StopSound(const std::string& name);
    void SetVolume(const std::string& name, float volume);

    void SetMasterVolume(float volume);

    void Terminate(); 
private:
    FMOD::System* mSystem = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> mSounds;
    std::unordered_map<std::string, FMOD::Channel*> mChannels;
};
