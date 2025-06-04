#include "pch.h"
#include "MaterialLoader.h"

MaterialFileLoader::MaterialFileLoader(std::shared_ptr<RenderManager> renderManager) : mRenderManager(renderManager) {
} 

void MaterialFileLoader::Load() {
    const std::filesystem::path materialDir { "Resources/Materials" };

    for (const auto& entry : std::filesystem::directory_iterator(materialDir)) {
        if (entry.is_regular_file()) {
			MaterialFileLoader::LoadMaterial(entry.path());
        }
    }
}

void MaterialFileLoader::LoadMaterial(const std::filesystem::path& materialFilePath) {
    std::ifstream file(materialFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open material file: " + materialFilePath.string());
    }

    MaterialConstants mat{};
    std::string line;
    std::string currentSection;

    int texIndex = 0;
    UINT* currentTexArray = nullptr;

    auto parseColor = [](const std::string& line)
        {
            std::istringstream iss(line);
            float r, g, b, a;
            iss >> r >> g >> b >> a;
            return SimpleMath::Color(r, g, b, a);
        };

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("DiffuseColor:") == 0) {
            mat.mDiffuseColor = parseColor(line.substr(strlen("DiffuseColor:")));
        }
        else if (line.find("SpecularColor:") == 0) {
            mat.mSpecularColor = parseColor(line.substr(strlen("SpecularColor:")));
        }
        else if (line.find("EmissiveColor:") == 0) {
            mat.mEmissiveColor = parseColor(line.substr(strlen("EmissiveColor:")));
        }
        else if (line.find("DiffuseTextures:") == 0) {
            currentSection = "DiffuseTextures";
            currentTexArray = mat.mDiffuseTexture;
            texIndex = 0;
        }
        else if (line.find("SpecularTextures:") == 0) {
            currentSection = "SpecularTextures";
            currentTexArray = mat.mSpecularTexture;
            texIndex = 0;
        }
        else if (line.find("EmissiveTextures:") == 0) {
            currentSection = "EmissiveTextures";
            currentTexArray = mat.mEmissiveTexture;
            texIndex = 0;
        }
        else if (line.find("MetalicTextures:") == 0) {
            currentSection = "MetalicTextures";
            currentTexArray = mat.mMetalicTexture;
            texIndex = 0;
        }
        else if (line.find("NormalTextures:") == 0) {
            currentSection = "NormalTextures";
            currentTexArray = mat.mNormalTexture;
            texIndex = 0;
        }
        else if (line.find("AlphaTextures:") == 0) {
            currentSection = "AlphaTextures";
            currentTexArray = mat.mAlphaTexture;
            texIndex = 0;
        }
        else if (!line.empty() && currentTexArray != nullptr && texIndex < 8) {
            currentTexArray[texIndex++] = mRenderManager->GetTextureManager().GetTexture(line);
        }
    }

    file.close();
       
    mRenderManager->GetMaterialManager().CreateMaterial(materialFilePath.stem().string(), mat); 
    
}
