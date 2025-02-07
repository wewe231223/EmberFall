#pragma once 
#include <memory>
#include <numeric>
#include "../Core/Shader.h"
#include "../Utility/DirectXInclude.h"

class MaterialConstants {
	SimpleMath::Color mDiffuseColor{};
	SimpleMath::Color mSpecularColor{};
	SimpleMath::Color mEmissiveColor{};

	UINT mDiffuseTexture[3]{};
	UINT mSpecularTexture[3]{};
	UINT mMetalicTexture[3]{};
	UINT mEmissiveTexture[3]{};
	UINT mNormalTexture[3]{};
	UINT mAlphaTexture[3]{};
};


class Material {
public:
	Material();
	~Material();
public:

private:
	std::shared_ptr<GraphicsShaderBase> mShader{ nullptr };
};