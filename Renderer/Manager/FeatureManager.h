#pragma once 
#include "../Utility/Defines.h"

class FeatureManager {
public:
	FeatureManager() = default;
	~FeatureManager() = default;

	FeatureManager(const FeatureManager& other) = default;
	FeatureManager& operator=(const FeatureManager& other) = default;

	FeatureManager(FeatureManager&& other) = default;
	FeatureManager& operator=(FeatureManager&& other) = default;

public:
	void SetFixedFeatures(Features& f);
	void SetFeature(Features& f); 
	void swap(); 

	void Render(); 

	Features& GetCurrentFeature();
public:
	Features mFixedFeatures{};

	Features mCurrentFeatures{}; 
	Features mNextFeatures{};
};