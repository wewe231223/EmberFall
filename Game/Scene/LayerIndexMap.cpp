#include "pch.h"
#include "LayerIndexMap.h"

bool LayerIndexMap::LoadFromFile(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.read(reinterpret_cast<char*>(&mXCount), sizeof(int));
	file.read(reinterpret_cast<char*>(&mZCount), sizeof(int));
	file.read(reinterpret_cast<char*>(&mSampleInterval), sizeof(float));
	file.read(reinterpret_cast<char*>(&mTerrainWidth), sizeof(float));
	file.read(reinterpret_cast<char*>(&mTerrainLength), sizeof(float));

	size_t dataSize = static_cast<size_t>(mXCount) * static_cast<size_t>(mZCount);
	mData.resize(dataSize);
	file.read(reinterpret_cast<char*>(mData.data()), dataSize);
	file.close();

	return true;
}

UINT LayerIndexMap::GetLayerIndexAtPosition(const DirectX::SimpleMath::Vector3& worldPos) const {
	float halfWidth = mTerrainWidth * 0.5f;
	float halfLength = mTerrainLength * 0.5f;

	float relativeX = worldPos.x + halfWidth;
	float relativeZ = worldPos.z + halfLength;

	if (relativeX < 0 || relativeZ < 0 || relativeX >= mTerrainWidth || relativeZ >= mTerrainLength) {
		return -1;
	}

	int xIndex = static_cast<int>(relativeX / mSampleInterval);
	int zIndex = static_cast<int>(relativeZ / mSampleInterval);

	if (xIndex < 0 || xIndex >= mXCount || zIndex < 0 || zIndex >= mZCount) {
		return -1;
	}

	size_t index = static_cast<size_t>(zIndex) * mXCount + xIndex;
	return static_cast<int>(mData[index]);
}