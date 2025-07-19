#pragma once 
#include "../Utility/DirectXInclude.h"

class LayerIndexMap {
public:
	LayerIndexMap() = default;
	~LayerIndexMap() = default;

public:
	bool LoadFromFile(const std::string& filePath);
	UINT GetLayerIndexAtPosition(const DirectX::SimpleMath::Vector3& worldPos) const;

private:
	std::vector<uint8_t> mData;
	int mXCount = 0;
	int mZCount = 0;
	float mSampleInterval = 0.1f;
	float mTerrainWidth = 0.f;
	float mTerrainLength = 0.f;
};
