#pragma once 
#include <string>
#include <filesystem>
#include <unordered_map>
#include "../Utility/DirectXInclude.h"

class ColliderBaker {
public:
	ColliderBaker() = default;
	~ColliderBaker() = default;
public:
	void Load(const std::filesystem::path& path);
	void Bake();

	void CreateBox(const std::string& name, const DirectX::BoundingBox& box);
	DirectX::BoundingBox& GetBox(const std::string& name);
private:
	std::unordered_map<std::string, DirectX::BoundingBox> mBoxes{}; 
};
