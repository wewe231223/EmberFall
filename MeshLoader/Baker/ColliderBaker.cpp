#include <fstream>
#include "ColliderBaker.h"
#include "../Utility/Crash.h"

void ColliderBaker::Load(const std::filesystem::path& path) {
    std::ifstream inFile{ path, std::ios::binary };

    Crash(bool(inFile));

    size_t mapSize;
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(size_t));

    for (size_t i = 0; i < mapSize; ++i) {
        size_t keySize;
        inFile.read(reinterpret_cast<char*>(&keySize), sizeof(size_t)); 

        std::string key(keySize, '\0');
        inFile.read(&key[0], keySize); 

        DirectX::BoundingBox box;
        inFile.read(reinterpret_cast<char*>(&box.Center), sizeof(DirectX::XMFLOAT3)); 
        inFile.read(reinterpret_cast<char*>(&box.Extents), sizeof(DirectX::XMFLOAT3)); 

        mBoxes[key] = box;
    }
}

void ColliderBaker::Bake() {
    std::ofstream out{ "Collider.bin" , std::ios::binary};

    size_t mapSize = mBoxes.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(size_t));

    for (const auto& [key, box] : mBoxes) {
        size_t keySize = key.size();
        out.write(reinterpret_cast<const char*>(&keySize), sizeof(size_t)); 
        out.write(key.data(), keySize); 
        out.write(reinterpret_cast<const char*>(&box.Center), sizeof(DirectX::XMFLOAT3)); 
        out.write(reinterpret_cast<const char*>(&box.Extents), sizeof(DirectX::XMFLOAT3));
    }

    out.close(); 
}

void ColliderBaker::CreateBox(const std::string& name, const DirectX::BoundingBox& box) {
    mBoxes[name] = box;
}

DirectX::BoundingBox& ColliderBaker::GetBox(const std::string& name) {
    auto it = mBoxes.find(name);
    Crash(it != mBoxes.end());

    return it->second;
}
