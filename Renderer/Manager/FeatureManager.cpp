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
    ImGui::Begin("Feature Manager");

    auto drawFeatureCheckbox = [](const char* label, bool& value, bool enabled) {
        if (!enabled)
            ImGui::BeginDisabled();
        ImGui::Checkbox(label, &value);
        if (!enabled)
            ImGui::EndDisabled();
        };

    drawFeatureCheckbox("Particle", mNextFeatures.Particle, mFixedFeatures.Particle);
    drawFeatureCheckbox("Grass", mNextFeatures.Grass, mFixedFeatures.Grass);
    drawFeatureCheckbox("Shadow", mNextFeatures.Shadow, mFixedFeatures.Shadow);
    drawFeatureCheckbox("Bloom", mNextFeatures.Bloom, mFixedFeatures.Bloom);

    ImGui::End();
}

Features& FeatureManager::GetCurrentFeature() {
	return mCurrentFeatures; 
}
