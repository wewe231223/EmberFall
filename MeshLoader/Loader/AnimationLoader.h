#pragma once 
#include <filesystem>
#include <fstream>
#include "../MeshLoader/Base/MeshData.h"
#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "External/lib/release/assimp-vc143-mt.lib")
#endif // DEBUG
#include "../External/Include/assimp/Importer.hpp"
#include "../External/Include/assimp/scene.h"
#include "../External/Include/assimp/postprocess.h"
#include "../MeshLoader/Base/AnimationData.h"


namespace Legacy {
	class AnimationLoader {
	public:
		AnimationLoader() = default;
		~AnimationLoader() = default;
	public:
		AnimationClip Load(const std::filesystem::path& path, UINT animIndex = 0);
	private:
		std::shared_ptr<BoneNode> buildHierarchy(const aiNode* node);
	private:
		Assimp::Importer mImporter{};
	};
}



class AnimationLoader {
	static constexpr const char* BINARY_PATH = "Resources/Assets/Binarys";
public:
	AnimationLoader() = default;
	~AnimationLoader() = default;
public:
	void Load(const std::filesystem::path& path);
	
	AnimationClip* GetClip(UINT index);
	UINT GetBoneIndex(const std::string& name);
private:
	bool CheckBinary(const std::filesystem::path& path);

	AnimationClip LoadClip(UINT animIndex = 0);
	std::shared_ptr<BoneNode> BuildNode(const aiNode* node, const std::unordered_map<std::string, UINT>& boneMap);
private:
	std::vector<AnimationClip> mClips{};

	std::unordered_map<std::string, UINT> mBoneIndexMap{}; 

	Assimp::Importer mImporter{};
	const aiScene* mScene{}; 
};


class AnimationSerializer {
public:
	explicit AnimationSerializer(const std::filesystem::path& fileName) : mFile(fileName, std::ios::binary) {}
	~AnimationSerializer() = default;
public:
	void Serialize(const std::unordered_map<std::string, UINT>& boneIndexMap);
	void Serialze(const std::vector<AnimationClip>& clips);
private:
	void SerializeClip(const AnimationClip& clip);

	void Write(const float& value);
	void Write(const double& value);
	void Write(const DirectX::SimpleMath::Matrix& value);
	void Write(const UINT& value);

	void Write(const std::string& value);

	template<typename T>
	void WriteVector(const std::vector<T>& value);

	void WriteVector3List(const std::vector<std::pair<double, DirectX::SimpleMath::Vector3>>& value);
	void WriteQuaternionList(const std::vector<std::pair<double, DirectX::SimpleMath::Quaternion>>& value);

	void WriteMap(const std::unordered_map<UINT, BoneAnimation>& value);

	void WriteBoneNode(const std::shared_ptr<BoneNode>& node);
private:
	std::ofstream mFile{};
};

template<typename T>
inline void AnimationSerializer::WriteVector(const std::vector<T>& value) {
	size_t size = value.size();
	mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
	mFile.write(reinterpret_cast<const char*>(value.data()), size * sizeof(T));
}



class AnimationDeserializer {
public:
	explicit AnimationDeserializer(const std::filesystem::path& fileName) : mFile(fileName, std::ios::binary) {}
	~AnimationDeserializer() = default;
public:
	std::unordered_map<std::string, UINT> DeserializeBoneIndexMap();
	std::vector<AnimationClip> DeserializeClips();
private:
	AnimationClip DeserializeClip();

	void Read(float& value);
	void Read(double& value);
	void Read(DirectX::SimpleMath::Matrix& value);
	void Read(UINT& value);

	void Read(std::string& value);

	template<typename T>
	void ReadVector(std::vector<T>& value);

	void ReadVector3List(std::vector<std::pair<double, DirectX::SimpleMath::Vector3>>& value);
	void ReadQuaternionList(std::vector<std::pair<double, DirectX::SimpleMath::Quaternion>>& value);

	void ReadMap(std::unordered_map<UINT, BoneAnimation>& value);

	std::shared_ptr<BoneNode> ReadBoneNode();
private:
	std::ifstream mFile{};
};

template<typename T>
inline void AnimationDeserializer::ReadVector(std::vector<T>& value) {
	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));
	
	value.resize(size);
	mFile.read(reinterpret_cast<char*>(value.data()), size * sizeof(T));
}


template <typename T>
inline void OffsetKeyTimes(std::vector<std::pair<double, T>>& keys, double offset) 
{
	for (auto& key : keys) {
		key.first += offset;
	}
}

inline AnimationClip AppendAnimationClips(const AnimationClip& clipA, const AnimationClip& clipB) 
{
	AnimationClip merged;

	merged.duration = clipA.duration + clipB.duration;
	merged.ticksPerSecond = clipA.ticksPerSecond;
	merged.globalInverseTransform = clipA.globalInverseTransform;
	merged.boneOffsetMatrices = clipA.boneOffsetMatrices;
	merged.root = clipA.root;
	merged.boneAnimations = clipA.boneAnimations;

	for (const auto& pair : clipB.boneAnimations) {
		unsigned int boneIndex = pair.first;
		BoneAnimation animB = pair.second;  

		OffsetKeyTimes(animB.positionKey, clipA.duration);
		OffsetKeyTimes(animB.rotationKey, clipA.duration);
		OffsetKeyTimes(animB.scalingKey, clipA.duration);

		auto it = merged.boneAnimations.find(boneIndex);
		if (it != merged.boneAnimations.end()) {
			it->second.positionKey.insert(it->second.positionKey.end(), animB.positionKey.begin(), animB.positionKey.end());
			it->second.rotationKey.insert(it->second.rotationKey.end(), animB.rotationKey.begin(), animB.rotationKey.end());
			it->second.scalingKey.insert(it->second.scalingKey.end(), animB.scalingKey.begin(), animB.scalingKey.end());
		}
		else {
			merged.boneAnimations[boneIndex] = animB;
		}
	}

	return merged;
}