#include "pch.h"
#include "Scene.h"
#include "../Renderer/Core/Renderer.h"


Scene::Scene(std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers) {
	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);


}

void Scene::Update()
{
}
