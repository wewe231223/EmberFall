#include "pch.h"
#include "BoundingBoxImporter.h"

bool BoundingBoxImporter::LoadFromFile(const std::filesystem::path& filePath) {
    std::ifstream in{ filePath, std::ios::binary };
    if (not in) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Bounding Box File is not exists. path: {}", filePath.string());
        return false;
    }

    size_t numEntries{ };
    in.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));

    std::string key{ };
    DirectX::BoundingBox box{ };
    for (size_t i = 0; i < numEntries; ++i) {
        size_t keyLength{};
        in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));

        key.resize(keyLength, '\0');
        in.read(key.data(), keyLength);
        in.read(reinterpret_cast<char*>(&box), sizeof(DirectX::BoundingBox));

        mBoundingBoxes[key] = box;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Bounding Box File Load Success", filePath.string());
    return true;
}

DirectX::BoundingBox BoundingBoxImporter::GetBoundingBox(const std::string& key) {
    if (not mBoundingBoxes.contains(key)) {
        return DirectX::BoundingBox{ };
    }

    return mBoundingBoxes[key];
}
