#include "pch.h"
#include "FeatureManager.h"

void FeatureManager::SetFixedFeatures(Features& f) {
	mFixedFeatures = f; 
}

void FeatureManager::SetFeature(Features& f) {
	mNextFeatures = f; 
}

void FeatureManager::swap() {
	mCurrentFeatures = mNextFeatures;
}

void FeatureManager::Render() {

}

Features& FeatureManager::GetCurrentFeature() {
	return mCurrentFeatures; 
}
