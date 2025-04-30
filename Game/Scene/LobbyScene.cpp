#include "pch.h"
#include "LobbyScene.h"
#include "../MeshLoader/Loader/MeshLoader.h"


LobbyScene::LobbyScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation) {
	mRenderManager = renderMgr;

	mCamera = Camera(mainCamLocation);
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 0.f, 2.f, 0.f };
	cameraTransform.Look({ 1.f,0.f,0.f });
}

LobbyScene::~LobbyScene() {
}

void LobbyScene::Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	LobbyScene::BuildShader(device);
	LobbyScene::BuildMesh(device, commandList);
	LobbyScene::BuildMaterial();
	LobbyScene::BuildLobbyObject(); 
	LobbyScene::BuildSwordMan();
	LobbyScene::BuildArcher();
	LobbyScene::BuildMage();
	LobbyScene::BuildShieldMan();
	LobbyScene::BuildDemon();
	LobbyScene::BuildEquipmentObject();
	LobbyScene::BuildPlayerPrefab();
	LobbyScene::BuildPlayerNameTextBlock(); 

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SkyBoxMaterial");

	mRenderManager->GetLightingManager().ClearLight(commandList);

	mPlayers[0] = mPlayerPreFabs["SwordMan"].Clone();
	mPlayers[1] = mPlayerPreFabs["ShieldMan"].Clone();
	mPlayers[2] = mPlayerPreFabs["Archer"].Clone();
	mPlayers[3] = mPlayerPreFabs["Mage"].Clone();

	constexpr float Interval = 2.f; 
	mPlayers[0].GetTransform().SetPosition({ -1.f * Interval * 1.5f , 0.f, 5.f });
	mPlayers[1].GetTransform().SetPosition({ -1.f * Interval * 0.5f , 0.f, 5.f });
	mPlayers[2].GetTransform().SetPosition({  1.f * Interval * 0.5f , 0.f, 5.f });
	mPlayers[3].GetTransform().SetPosition({  1.f * Interval * 1.5f , 0.f, 5.f });

	// mRenderManager->GetLightingManager().CreatePointLight(commandList, { -1.f * Interval * 0.5f , 0.f, 5.f }, { 1.f, 1.f, 1.f, 1.f });
	for (int i = 0; i < 4; ++i) {
		auto& light = mRenderManager->GetLightingManager().GetLight(i);
		//light.mType = LightType::Spot; 
		light.Position = mPlayers[i].GetTransform().GetPosition();
		light.Position.y += 10.f;
		light.Diffuse = { 1.f, 1.f, 1.f, 1.f };
		light.Direction = { 0.f, -1.f, 0.f };
		light.Range = 20.f;
		light.InnerAngle = 3.f;
		light.OuterAngle = 5.f;
	}




	mPlayers[4] = mPlayerPreFabs["Demon"].Clone();
	mPlayers[4].GetTransform().SetPosition({ 0.f, 0.f, -10.f });
	mPlayers[4].GetTransform().Rotate(0.f, 110.f, 0.f);

	{
		auto& light = mRenderManager->GetLightingManager().GetLight(4);
		//light.mType = LightType::Spot;
		light.Position = mPlayers[4].GetTransform().GetPosition();
		light.Position.y += 20.f;
		light.Diffuse = { 1.f, 1.f, 1.f, 1.f };
		light.Direction = { 0.f, -1.f, 0.f };
		light.Range = 100.f;
		light.InnerAngle = 5.f;
		light.OuterAngle = 10.f;
	}


	mCamera.GetTransform().Look({ 0.f,0.f, 15.f });

	auto packet = FbsPacketFactory::PlayerEnterInGame(gClientCore->GetSessionId());
	gClientCore->Send(packet);
}

void LobbyScene::ProcessNetwork() {

}

void LobbyScene::Update() {

	if (Input.GetKeyboardTracker().pressed.Tab) {
		mCamera.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(180.f), 0.f);
		if (mPlayerSelected == 4) { // 인간 진영 선택 
			mRenderManager->GetLightingManager().GetLight(mPlayerSelected).mType = LightType::None;
			mPlayerSelected = 0; 

			for (auto i = 0; i < mPlayers.size() - 1; ++i) {
				mPlayerNameTextBlock[i]->SetActiveState(true);
			}

			mPlayerNameTextBlock[4]->SetActiveState(false);
		}
		else { // 악마 진영 선택 
			mRenderManager->GetLightingManager().GetLight(mPlayerSelected).mType = LightType::None;
			mPlayerSelected = 4; 

			for (auto i = 0; i < mPlayers.size() - 1; ++i) {
				mPlayerNameTextBlock[i]->SetActiveState(false);
			}
			mPlayerNameTextBlock[4]->SetActiveState(true);
		}
	}

	//mRenderManager->GetLightingManager().GetLight(mPlayerSelected).mType = LightType::Spot;

	mCamera.UpdateBuffer(); 

	for (auto& object : mLobbyProps) {
		object.UpdateShaderVariables();
		auto [mesh, shader, modelContext] = object.GetRenderData();
		mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(shader, mesh, modelContext);
	}

	for (auto& player : mPlayers) {
		if (player.GetActiveState() == false) {
			continue;
		}
		player.Update(mRenderManager->GetMeshRenderManager());
	}

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition(); 
	mSkyBox.UpdateShaderVariables();

	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);
}

void LobbyScene::SendNetwork() {
}

void LobbyScene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);

	mMeshMap["Plane"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Plane, 50);

	MeshLoader Loader{};
	MeshData data{};

	data = Loader.Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
	mMeshMap["SwordMan"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/Demon.glb");
	mMeshMap["Demon"] = std::make_unique<Mesh>(device, commandList, data);


	data = Loader.Load("Resources/Assets/Weapon/sword/LongSword.glb");
	mMeshMap["Sword"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/great_sword/Sword.glb");
	mMeshMap["GreatSword"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/Bow.glb");
	mMeshMap["Bow"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/Arrow.glb");
	mMeshMap["Arrow"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/quiver.glb");
	mMeshMap["Quiver"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Shield/Shield.glb");
	mMeshMap["Shield"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/DemonWeapon.glb");
	mMeshMap["DemonWeapon"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/DemonCloth.glb");
	mMeshMap["DemonCloth"] = std::make_unique<Mesh>(device, commandList, data);
}

void LobbyScene::BuildMaterial() {
	MaterialConstants mat{};

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Front_0");
	mat.mDiffuseTexture[1] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Back_0");
	mat.mDiffuseTexture[2] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Top_0");
	mat.mDiffuseTexture[3] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Bottom_0");
	mat.mDiffuseTexture[4] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Left_0");
	mat.mDiffuseTexture[5] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Right_0");
	mRenderManager->GetMaterialManager().CreateMaterial("SkyBoxMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("dirt_2");
	mRenderManager->GetMaterialManager().CreateMaterial("GroundMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Paladin_diffuse");
	mRenderManager->GetMaterialManager().CreateMaterial("HumanMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("SwordA_v004_Default_AlbedoTransparency");
	mRenderManager->GetMaterialManager().CreateMaterial("SwordMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("sword_base");
	mRenderManager->GetMaterialManager().CreateMaterial("GreatSwordMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Bow_DIFF");
	mRenderManager->GetMaterialManager().CreateMaterial("BowMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Quiver_baseColor");
	mRenderManager->GetMaterialManager().CreateMaterial("QuiverMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Body_Albedo_Skin_3");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Axe_Albedo_Skin_1");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonWeaponMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Clothes_Albedo_Skin_1");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonClothMaterial", mat);
}


void LobbyScene::BuildShader(ComPtr<ID3D12Device> device) {
	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<SkyBoxShader>();
	shader->CreateShader(device);
	mShaderMap["SkyBoxShader"] = std::move(shader);

	shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);
	mShaderMap["StandardShader"] = std::move(shader);

	shader = std::make_unique<SkinnedShader>();
	shader->CreateShader(device);
	mShaderMap["SkinnedShader"] = std::move(shader);
}

void LobbyScene::BuildLobbyObject() {
	{
		auto& object = mLobbyProps.emplace_back();
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Plane"].get();
		object.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("GroundMaterial");

		object.SetActiveState(true);
	}
}

void LobbyScene::BuildSwordMan() {
	mAnimationMap["LongSword"].Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
	auto& loader = mAnimationMap["LongSword"];

	AnimatorGraph::AnimationState idle{};
	 
	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle"; 

	mSwordManAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildArcher() {
	mAnimationMap["Archer"].Load("Resources/Assets/Knight/Archer/Archer.glb");
	auto& loader = mAnimationMap["Archer"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mArcherAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildMage() {
	mAnimationMap["Magician"].Load("Resources/Assets/Knight/Mage/Magician.glb");
	auto& loader = mAnimationMap["Magician"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mMageAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildShieldMan() {
	mAnimationMap["ShieldMan"].Load("Resources/Assets/Knight/ShieldMan/ShieldMan.glb");
	auto& loader = mAnimationMap["ShieldMan"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mShieldManAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildDemon() {
	mAnimationMap["Demon"].Load("Resources/Assets/Demon/Demon.glb");
	auto& loader = mAnimationMap["Demon"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mDemonAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildEquipmentObject() {
	{
		mEquipments["Sword"] = EquipmentObject{};
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Sword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SwordMaterial");
		mEquipments["Sword"].mEquipJointIndex = 36;
		mEquipments["Sword"].SetActiveState(true);
	}


	{
		mEquipments["GreatSword"] = EquipmentObject{};
		mEquipments["GreatSword"].mMesh = mMeshMap["GreatSword"].get();
		mEquipments["GreatSword"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["GreatSword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("GreatSwordMaterial");
		mEquipments["GreatSword"].mEquipJointIndex = 36;
		mEquipments["GreatSword"].SetActiveState(true);
	}

	{
		mEquipments["Bow"] = EquipmentObject{};
		mEquipments["Bow"].mMesh = mMeshMap["Bow"].get();
		mEquipments["Bow"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Bow"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("BowMaterial");
		mEquipments["Bow"].mEquipJointIndex = 12;
		mEquipments["Bow"].SetActiveState(true);
	}

	{
		mEquipments["Quiver"] = EquipmentObject{};
		mEquipments["Quiver"].mMesh = mMeshMap["Quiver"].get();
		mEquipments["Quiver"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Quiver"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("QuiverMaterial");
		mEquipments["Quiver"].mEquipJointIndex = 3;
		mEquipments["Quiver"].SetActiveState(true);
	}

	{
		mEquipments["Shield"] = EquipmentObject{};
		mEquipments["Shield"].mMesh = mMeshMap["Shield"].get();
		mEquipments["Shield"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Shield"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial");
		mEquipments["Shield"].mEquipJointIndex = 11;
		mEquipments["Shield"].SetActiveState(true);
	}

	{
		mEquipments["DemonCloth"] = EquipmentObject{};
		mEquipments["DemonCloth"].mMesh = mMeshMap["DemonCloth"].get();
		mEquipments["DemonCloth"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["DemonCloth"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonClothMaterial");
		mEquipments["DemonCloth"].mEquipJointIndex = 0;
		mEquipments["DemonCloth"].SetActiveState(true);
	}

	{
		mEquipments["DemonWeapon"] = EquipmentObject{};
		mEquipments["DemonWeapon"].mMesh = mMeshMap["DemonWeapon"].get();
		mEquipments["DemonWeapon"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["DemonWeapon"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonWeaponMaterial");
		mEquipments["DemonWeapon"].mEquipJointIndex = 28;
		mEquipments["DemonWeapon"].SetActiveState(true);
	}
}

void LobbyScene::BuildPlayerPrefab() {
	mPlayerPreFabs["SwordMan"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mSwordManAnimationController };
	mPlayerPreFabs["SwordMan"].AddEquipment(mEquipments["GreatSword"]);
	
	mPlayerPreFabs["ShieldMan"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mShieldManAnimationController };
	mPlayerPreFabs["ShieldMan"].AddEquipment(mEquipments["Sword"].Clone());
	mPlayerPreFabs["ShieldMan"].AddEquipment(mEquipments["Shield"].Clone());

	mPlayerPreFabs["Archer"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mArcherAnimationController };
	mPlayerPreFabs["Archer"].AddEquipment(mEquipments["Bow"].Clone());
	mPlayerPreFabs["Archer"].AddEquipment(mEquipments["Quiver"].Clone());

	mPlayerPreFabs["Mage"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mMageAnimationController };

	mPlayerPreFabs["Demon"] = Player{ mMeshMap["Demon"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("DemonMaterial"), mDemonAnimationController };
	mPlayerPreFabs["Demon"].AddEquipment(mEquipments["DemonWeapon"].Clone());
	mPlayerPreFabs["Demon"].AddEquipment(mEquipments["DemonCloth"].Clone());
}

void LobbyScene::BuildPlayerNameTextBlock() {
	const float xInterval = 370.f;
	const float xPadding = 350.f; 

	const float y = 830.f;

	for (auto i = 0; i < mPlayers.size() - 1; ++i) {
		auto& block = mPlayerNameTextBlock[i];
		block = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ xPadding + xInterval * i , y, xPadding + xInterval * i + 100.f, y + 100.f }, StringColor::White, "NotoSansKR");
		block->GetText() = L"Player" + std::to_wstring(i);
		block->SetActiveState(true);
	}

	auto& block = mPlayerNameTextBlock[4];
	block = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 760.f, 630.f, 760.f + xInterval, 730.f }, StringColor::White, "NotoSansKR");
	block->GetText() = L"Player" + std::to_wstring(4);
	block->SetActiveState(false);
}
