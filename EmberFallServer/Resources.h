#pragma once

const char* const ANIM_KEY_ARCHER = "Archer";
const char* const ANIM_KEY_LONGSWORD_MAN = "LongSword";
const char* const ANIM_KEY_MAGICIAN = "Magician";
const char* const ANIM_KEY_SHIELD_MAN = "ShieldMan";
const char* const ANIM_KEY_MONSTER = "Monster";
const char* const ANIM_KEY_DEMON = "Demon";

const char* const ENTITY_KEY_HUMAN = "Human";
const char* const ENTITY_KEY_IMP = "Imp";
const char* const ENTITY_KEY_DEMON = "Demon";
const char* const ENTITY_KEY_GREATSWORD = "GreatSword";
const char* const ENTITY_KEY_SWORD = "Sword";
const char* const ENTITY_KEY_ARROW = "Arrow";

struct EntityInfo {
    DirectX::BoundingOrientedBox bb;
    SimpleMath::Vector3 pivot;
    Packets::EntityType entity;
};

struct EnvironmentsInfo {
    GameProtocol::EnvironmentType envType;
    DirectX::BoundingOrientedBox bb;
    SimpleMath::Vector3 pivot;
};

inline constexpr size_t NUM_OF_ANIM = Packets::AnimationState_MAX + 1;

struct AnimationInfos {
    float durations[NUM_OF_ANIM];
};

class ResourceManager {
public:
    // delete constructor
    // only use static method
    ResourceManager() = delete;
    ~ResourceManager() = delete;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

public:
    static void LoadEnvFromFile(const std::filesystem::path& path);
    static void LoadAnimationFromFile(const std::filesystem::path& path);
    static void LoadEntityFromFile(const std::filesystem::path& path);

    static const EntityInfo& GetEntityInfo(const std::string& name);
    static const AnimationInfos& GetAnimInfo(const std::string& key);
    static const EnvironmentsInfo& GetEnvInfo(GameProtocol::EnvironmentType envType);

private:
    inline static std::unordered_map<std::string, AnimationInfos> mAnimInfos;
    inline static std::unordered_map<std::string, EntityInfo> mEntityInfos;
    inline static std::unordered_map<GameProtocol::EnvironmentType, EnvironmentsInfo> mEnvInfos;
};