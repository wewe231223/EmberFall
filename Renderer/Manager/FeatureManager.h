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

public:
	Features mCurrentFeatures{}; 

};