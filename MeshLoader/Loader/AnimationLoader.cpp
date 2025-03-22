#include <functional>
#include "AnimationLoader.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"

#ifdef max 
#undef max 
#endif

#pragma region Legacy
namespace Legacy {
    AnimationClip AnimationLoader::Load(const std::filesystem::path& path, UINT animIndex) {
        Assimp::Importer importer{};
        const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

        CrashExp(scene != nullptr, "Failed To Load Animation!");
        CrashExp((!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)), "Failed To Load Animation!");
        CrashExp((scene->mRootNode != nullptr), "Failed To Load Animation!");

        if (!scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !scene->mRootNode) {
            Console.Log("Animation Load Failed : {}", LogType::Error, mImporter.GetErrorString());
            Crash("Animation Load Failed");
        }

        CrashExp(scene->mNumAnimations != 0, "Any Animation Data not Found");
        CrashExp(animIndex < scene->mNumAnimations, "Invalid Animation Index");

        aiAnimation* aiAnim = scene->mAnimations[animIndex];
        aiNode* rootNode = scene->mRootNode;

        AnimationClip clip;
        clip.duration = aiAnim->mDuration;
        clip.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0f;

        std::unordered_map<std::string, UINT> boneMap{};

        UINT boneNumbers{ 0 };

        for (UINT i = 0; i < scene->mNumMeshes; ++i) {

            auto mesh = scene->mMeshes[i];
            for (UINT j = 0; j < mesh->mNumBones; ++j) {
                UINT boneIndex{ 0 };
                std::string boneName = mesh->mBones[j]->mName.C_Str();

                if (boneMap.find(boneName) == boneMap.end()) {
                    boneIndex = boneNumbers;
                    boneNumbers++;
                    boneMap[boneName] = boneIndex;

                    clip.boneOffsetMatrices.emplace_back();

                    auto offset = SimpleMath::Matrix{
                        mesh->mBones[j]->mOffsetMatrix.a1, mesh->mBones[j]->mOffsetMatrix.a2, mesh->mBones[j]->mOffsetMatrix.a3, mesh->mBones[j]->mOffsetMatrix.a4,
                        mesh->mBones[j]->mOffsetMatrix.b1, mesh->mBones[j]->mOffsetMatrix.b2, mesh->mBones[j]->mOffsetMatrix.b3, mesh->mBones[j]->mOffsetMatrix.b4,
                        mesh->mBones[j]->mOffsetMatrix.c1, mesh->mBones[j]->mOffsetMatrix.c2, mesh->mBones[j]->mOffsetMatrix.c3, mesh->mBones[j]->mOffsetMatrix.c4,
                        mesh->mBones[j]->mOffsetMatrix.d1, mesh->mBones[j]->mOffsetMatrix.d2, mesh->mBones[j]->mOffsetMatrix.d3, mesh->mBones[j]->mOffsetMatrix.d4
                    }.Transpose();


                    clip.boneOffsetMatrices[boneIndex] = offset;

                }
                else {
                    boneIndex = boneMap[boneName];
                }

            }
        }

        clip.boneIndexMap = boneMap;
        clip.root = buildHierarchy(rootNode);

        for (UINT i = 0; i < aiAnim->mNumChannels; ++i) {
            const aiNodeAnim* nodeAnim = aiAnim->mChannels[i];

            BoneAnimation boneAnim;

            boneAnim.boneName = nodeAnim->mNodeName.data;

            for (UINT j = 0; j < nodeAnim->mNumPositionKeys; ++j) {
                boneAnim.positionKey.emplace_back(nodeAnim->mPositionKeys[j].mTime, SimpleMath::Vector3(nodeAnim->mPositionKeys[j].mValue.x, nodeAnim->mPositionKeys[j].mValue.y, nodeAnim->mPositionKeys[j].mValue.z));
            }

            for (UINT j = 0; j < nodeAnim->mNumRotationKeys; ++j) {
                boneAnim.rotationKey.emplace_back(nodeAnim->mRotationKeys[j].mTime, SimpleMath::Quaternion(nodeAnim->mRotationKeys[j].mValue.x, nodeAnim->mRotationKeys[j].mValue.y, nodeAnim->mRotationKeys[j].mValue.z, nodeAnim->mRotationKeys[j].mValue.w));
            }

            for (UINT j = 0; j < nodeAnim->mNumScalingKeys; ++j) {
                boneAnim.scalingKey.emplace_back(nodeAnim->mScalingKeys[j].mTime, SimpleMath::Vector3(nodeAnim->mScalingKeys[j].mValue.x, nodeAnim->mScalingKeys[j].mValue.y, nodeAnim->mScalingKeys[j].mValue.z));
            }

            clip.boneAnimations[boneAnim.boneName] = boneAnim;
        }

        auto giTransform = scene->mRootNode->mTransformation.Inverse();

        clip.globalInverseTransform = SimpleMath::Matrix{
            giTransform.a1, giTransform.a2, giTransform.a3, giTransform.a4,
            giTransform.b1, giTransform.b2, giTransform.b3, giTransform.b4,
            giTransform.c1, giTransform.c2, giTransform.c3, giTransform.c4,
            giTransform.d1, giTransform.d2, giTransform.d3, giTransform.d4
        }.Transpose();

        return clip;

    }

    std::shared_ptr<BoneNode> AnimationLoader::buildHierarchy(const aiNode* node) {
        std::shared_ptr<BoneNode> newNode{ std::make_shared<BoneNode>() };
        newNode->name = node->mName.data;

        newNode->transformation = SimpleMath::Matrix{
            node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4,
            node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4,
            node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4,
            node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4
        }.Transpose();

        for (UINT i = 0; i < node->mNumChildren; ++i) {
            std::shared_ptr<BoneNode> child = buildHierarchy(node->mChildren[i]);
            newNode->children.emplace_back(child);
        }

        return newNode;
    }
}
#pragma endregion

AnimationClip AnimationLoader::LoadClip(UINT animIndex) {
    CrashExp((animIndex < mScene->mNumAnimations), "Invalid Animation Index");

    aiAnimation* aiAnim = mScene->mAnimations[animIndex];
    aiNode* rootNode = mScene->mRootNode;

    AnimationClip clip{};
    clip.duration = aiAnim->mDuration;
    clip.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0;

    UINT boneNumbers{ 0 };

    for (UINT i = 0; i < mScene->mNumMeshes; ++i) {
        auto mesh = mScene->mMeshes[i];
        for (UINT j = 0; j < mesh->mNumBones; ++j) {
            UINT boneIndex{ 0 };
            std::string boneName = mesh->mBones[j]->mName.C_Str();

            if (mBoneIndexMap.find(boneName) == mBoneIndexMap.end()) {
                boneIndex = boneNumbers;
                boneNumbers++;
                mBoneIndexMap[boneName] = boneIndex;

                clip.boneOffsetMatrices.emplace_back();
                auto offset = SimpleMath::Matrix{
                    mesh->mBones[j]->mOffsetMatrix.a1, mesh->mBones[j]->mOffsetMatrix.a2, mesh->mBones[j]->mOffsetMatrix.a3, mesh->mBones[j]->mOffsetMatrix.a4,
                    mesh->mBones[j]->mOffsetMatrix.b1, mesh->mBones[j]->mOffsetMatrix.b2, mesh->mBones[j]->mOffsetMatrix.b3, mesh->mBones[j]->mOffsetMatrix.b4,
                    mesh->mBones[j]->mOffsetMatrix.c1, mesh->mBones[j]->mOffsetMatrix.c2, mesh->mBones[j]->mOffsetMatrix.c3, mesh->mBones[j]->mOffsetMatrix.c4,
                    mesh->mBones[j]->mOffsetMatrix.d1, mesh->mBones[j]->mOffsetMatrix.d2, mesh->mBones[j]->mOffsetMatrix.d3, mesh->mBones[j]->mOffsetMatrix.d4
                }.Transpose();
                clip.boneOffsetMatrices[boneIndex] = offset;
            }        
        }
    }

    for (const auto& [boneName, boneIndex] : mBoneIndexMap) {
        clip.boneAnimations[boneIndex] = BoneAnimation{};
    }

    for (UINT i = 0; i < aiAnim->mNumChannels; ++i) {
            
        const aiNodeAnim* nodeAnim = aiAnim->mChannels[i];

        auto it = mBoneIndexMap.find(nodeAnim->mNodeName.data);
        if (it != mBoneIndexMap.end()) {
            UINT boneIndex = it->second;
            BoneAnimation& boneAnim = clip.boneAnimations[boneIndex];

            for (UINT j = 0; j < nodeAnim->mNumPositionKeys; ++j) {
                boneAnim.positionKey.emplace_back(nodeAnim->mPositionKeys[j].mTime, DirectX::XMFLOAT3(
                    nodeAnim->mPositionKeys[j].mValue.x,
                    nodeAnim->mPositionKeys[j].mValue.y,
                    nodeAnim->mPositionKeys[j].mValue.z));
            }

            for (UINT j = 0; j < nodeAnim->mNumRotationKeys; ++j) {
                boneAnim.rotationKey.emplace_back(nodeAnim->mRotationKeys[j].mTime, DirectX::XMFLOAT4(
                    nodeAnim->mRotationKeys[j].mValue.x,
                    nodeAnim->mRotationKeys[j].mValue.y,
                    nodeAnim->mRotationKeys[j].mValue.z,
                    nodeAnim->mRotationKeys[j].mValue.w));
            }

            for (UINT j = 0; j < nodeAnim->mNumScalingKeys; ++j) {
                boneAnim.scalingKey.emplace_back(nodeAnim->mScalingKeys[j].mTime, DirectX::XMFLOAT3(
                    nodeAnim->mScalingKeys[j].mValue.x,
                    nodeAnim->mScalingKeys[j].mValue.y,
                    nodeAnim->mScalingKeys[j].mValue.z));
            }
        }
    }

    std::erase_if(clip.boneAnimations, [](const std::pair<UINT, BoneAnimation>& pair) {
        return pair.second.positionKey.empty() and pair.second.rotationKey.empty() and pair.second.scalingKey.empty();
        });

    clip.root = BuildNode(rootNode, mBoneIndexMap);
        
    auto giTransform = mScene->mRootNode->mTransformation.Inverse();
    clip.globalInverseTransform = SimpleMath::Matrix{
        giTransform.a1, giTransform.a2, giTransform.a3, giTransform.a4,
        giTransform.b1, giTransform.b2, giTransform.b3, giTransform.b4,
        giTransform.c1, giTransform.c2, giTransform.c3, giTransform.c4,
        giTransform.d1, giTransform.d2, giTransform.d3, giTransform.d4
    }.Transpose();

        
    return clip;
}

void AnimationLoader::Load(const std::filesystem::path& path) {
    if (AnimationLoader::CheckBinary(path)) {
		AnimationDeserializer deserializer{ std::filesystem::path(BINARY_PATH) / (path.filename().stem().string() + ".bin") };
        mBoneIndexMap = deserializer.DeserializeBoneIndexMap(); 
		mClips = deserializer.DeserializeClips();
    }
    else {
        mScene = mImporter.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

        if (!mScene or mScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !mScene->mRootNode) {
            // Console.Log("Animation Load Failed : {}", LogType::Error, mImporter.GetErrorString());
            OutputDebugStringA("Animation Load Failed : ");
            OutputDebugStringA(mImporter.GetErrorString());
            OutputDebugStringA("\n");

            Crash("Animation Load Failed");
        }

        CrashExp(mScene->mNumAnimations != 0, "Any Animation Data not Found");

        for (UINT i = 0; i < mScene->mNumAnimations; ++i) {
            mBoneIndexMap.clear(); 
            mClips.emplace_back(LoadClip(i));
        }

        std::filesystem::path binaryPath = std::filesystem::path(BINARY_PATH) / (path.filename().stem().string() + ".bin");
        AnimationSerializer serializer{ binaryPath };
		
        serializer.Serialize(mBoneIndexMap);
        serializer.Serialze(mClips);
    }
}

AnimationClip* AnimationLoader::GetClip(UINT index) {
    return std::addressof(mClips[index]);
}

UINT AnimationLoader::GetBoneIndex(const std::string& name) {
    return mBoneIndexMap[name];
}

bool AnimationLoader::CheckBinary(const std::filesystem::path& path) {
    std::filesystem::path binaryPath = std::filesystem::path(BINARY_PATH) / (path.filename().stem().string() + ".bin" );
    return std::filesystem::exists(binaryPath);
}

std::shared_ptr<BoneNode> AnimationLoader::BuildNode(const aiNode* node, const std::unordered_map<std::string, UINT>& boneMap) {
    std::shared_ptr<BoneNode> newNode = std::make_shared<BoneNode>();

    auto it = boneMap.find(node->mName.data);

    newNode->index = (it != boneMap.end()) ? it->second : std::numeric_limits<UINT>::max();

    newNode->transformation = SimpleMath::Matrix{
        node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4,
        node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4,
        node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4,
        node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4
    }.Transpose();

    for (UINT i = 0; i < node->mNumChildren; ++i) {
        newNode->children.emplace_back(BuildNode(node->mChildren[i], boneMap));
    }

    return newNode;
}


#pragma region AnimationSerializer
void AnimationSerializer::Serialize(const std::unordered_map<std::string, UINT>& boneIndexMap) {
	size_t size = boneIndexMap.size();
	mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

	for (const auto& [key, value] : boneIndexMap) {
		Write(key);
		Write(value);
	}
}

void AnimationSerializer::Serialze(const std::vector<AnimationClip>& clips) {
	size_t size = clips.size();
	mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

    for (const auto& clip : clips) {
        SerializeClip(clip);
    }
}

void AnimationSerializer::SerializeClip(const AnimationClip& clip) {
    Write(clip.duration);
    Write(clip.ticksPerSecond);
    Write(clip.globalInverseTransform);

    WriteVector(clip.boneOffsetMatrices);
    WriteMap(clip.boneAnimations);
    WriteBoneNode(clip.root);
}

void AnimationSerializer::Write(const float& value) {
	mFile.write(reinterpret_cast<const char*>(&value), sizeof(float));
}

void AnimationSerializer::Write(const double& value) {
	mFile.write(reinterpret_cast<const char*>(&value), sizeof(double));
}

void AnimationSerializer::Write(const DirectX::SimpleMath::Matrix& value) {
	mFile.write(reinterpret_cast<const char*>(&value), sizeof(DirectX::SimpleMath::Matrix));
}

void AnimationSerializer::Write(const UINT& value) {
	mFile.write(reinterpret_cast<const char*>(&value), sizeof(UINT));
}

void AnimationSerializer::Write(const std::string& value) {
    size_t size = value.size();
    mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    mFile.write(value.data(), size);
}

void AnimationSerializer::WriteVector3List(const std::vector<std::pair<double, DirectX::SimpleMath::Vector3>>& value) {
    size_t size = value.size();
    mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    for (const auto& [time, value] : value) {
        Write(time);
        mFile.write(reinterpret_cast<const char*>(&value), sizeof(SimpleMath::Vector3));
    }
}

void AnimationSerializer::WriteQuaternionList(const std::vector<std::pair<double, DirectX::SimpleMath::Quaternion>>& value) {
	size_t size = value.size();
	mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
	for (const auto& [time, value] : value) {
		Write(time);
		mFile.write(reinterpret_cast<const char*>(&value), sizeof(SimpleMath::Quaternion));
	}
}

void AnimationSerializer::WriteMap(const std::unordered_map<UINT, BoneAnimation>& value) {
	size_t size = value.size();
	mFile.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
	for (const auto& [key, value] : value) {
		Write(key);
		Write(value.boneName);
		WriteVector3List(value.positionKey);
		WriteQuaternionList(value.rotationKey);
		WriteVector3List(value.scalingKey);
	}
}

void AnimationSerializer::WriteBoneNode(const std::shared_ptr<BoneNode>& node) {
    if (!node) {
        UINT nullMarker = UINT_MAX - 1;
        Write(nullMarker);
        return;
    }

    Write(node->index);
    Write(node->transformation);

    UINT numChildren = static_cast<UINT>(node->children.size());
    Write(numChildren);

    for (const auto& child : node->children) {
        WriteBoneNode(child);
    }
}
#pragma endregion 


#pragma region AnimationDeserializer
std::unordered_map<std::string, UINT> AnimationDeserializer::DeserializeBoneIndexMap() {
    std::unordered_map<std::string, UINT> boneIndexMap{};

    size_t size{};
    mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));

    for (size_t i = 0; i < size; ++i) {
        std::string key{};
        UINT value{};

        Read(key);
        Read(value);

        boneIndexMap[key] = value;
    }

    return boneIndexMap;
}

std::vector<AnimationClip> AnimationDeserializer::DeserializeClips() {
    std::vector<AnimationClip> clips{};

	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));

	for (size_t i = 0; i < size; ++i) {
		clips.emplace_back(DeserializeClip());
	}

    return clips;
}
AnimationClip AnimationDeserializer::DeserializeClip() {
    AnimationClip clip;

    Read(clip.duration);
    Read(clip.ticksPerSecond);
    Read(clip.globalInverseTransform);

    ReadVector(clip.boneOffsetMatrices);
    ReadMap(clip.boneAnimations);
    clip.root = ReadBoneNode();

    return clip;
}

void AnimationDeserializer::Read(float& value) {
	mFile.read(reinterpret_cast<char*>(&value), sizeof(float));
}

void AnimationDeserializer::Read(double& value) {
	mFile.read(reinterpret_cast<char*>(&value), sizeof(double));
}

void AnimationDeserializer::Read(DirectX::SimpleMath::Matrix& value) {
	mFile.read(reinterpret_cast<char*>(&value), sizeof(DirectX::SimpleMath::Matrix));
}

void AnimationDeserializer::Read(UINT& value) {
	mFile.read(reinterpret_cast<char*>(&value), sizeof(UINT));
}

void AnimationDeserializer::Read(std::string& value) {
	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));
	value.resize(size);
	mFile.read(value.data(), size);
}

void AnimationDeserializer::ReadVector3List(std::vector<std::pair<double, DirectX::SimpleMath::Vector3>>& value) {
	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));
	value.resize(size);
	for (auto& [time, vec] : value) {
		Read(time);
		mFile.read(reinterpret_cast<char*>(&vec), sizeof(DirectX::SimpleMath::Vector3));
	}
}

void AnimationDeserializer::ReadQuaternionList(std::vector<std::pair<double, DirectX::SimpleMath::Quaternion>>& value) {
	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));
	value.resize(size);
	for (auto& [time, quat] : value) {
		Read(time);
		mFile.read(reinterpret_cast<char*>(&quat), sizeof(DirectX::SimpleMath::Quaternion));
	}
}

void AnimationDeserializer::ReadMap(std::unordered_map<UINT, BoneAnimation>& value) {
	size_t size{};
	mFile.read(reinterpret_cast<char*>(&size), sizeof(size_t));
	for (size_t i = 0; i < size; ++i) {
		UINT key{};
		Read(key);
		
        BoneAnimation boneAnim;
		Read(boneAnim.boneName);
		
        ReadVector3List(boneAnim.positionKey);
		ReadQuaternionList(boneAnim.rotationKey);
		ReadVector3List(boneAnim.scalingKey);
		
        value[key] = boneAnim;
	}
}

std::shared_ptr<BoneNode> AnimationDeserializer::ReadBoneNode() {
	UINT index{};
	Read(index);

    if (index == UINT_MAX - 1) return nullptr;

	auto node = std::make_shared<BoneNode>();
	node->index = index;

	Read(node->transformation);

	UINT numChildren{};
	Read(numChildren);

	for (UINT i = 0; i < numChildren; ++i) {
		node->children.emplace_back(ReadBoneNode());
	}

    return node;
}
#pragma endregion