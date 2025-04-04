#pragma once

namespace EntryKeys {
    inline const std::filesystem::path DEFAULT_BOX_PATH{ "../Resources/Binarys/Collider/BoundingBoxes.bin" };
    inline const char* PLAYER_BOUNDING_BOX = "Human";
    inline const char* SWORD_BOUNDING_BOX = "LongSword";
}

class BoundingBoxImporter {
public:
    BoundingBoxImporter() = delete;
    ~BoundingBoxImporter() = delete;

public:
    static bool LoadFromFile(const std::filesystem::path& filePath=EntryKeys::DEFAULT_BOX_PATH);
    static DirectX::BoundingBox GetBoundingBox(const std::string& key);

private:
    inline static std::unordered_map<std::string, DirectX::BoundingBox> mBoundingBoxes;
};