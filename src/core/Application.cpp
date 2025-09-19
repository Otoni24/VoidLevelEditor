#include <imgui_internal.h>
#include "misc/cpp/imgui_stdlib.h"
#include <ImGuiFileDialog.h>
#include "core/Application.h"
#include "core/Utils.h"
#include "project/AssetManager.h"

Application::Application()
	: mWindow{ sf::VideoMode({1000, 680}), "Level Editor", sf::Style::Default },
	mTickClock{},
	mCleanCycleClock{},
	mCleanCycleInterval{},
	mIsFirstFrame{ true },
	mLevelCanvas({ 800, 600 }),
	mLevelView({0.f,0.f}, {1920.f, 1080.f}),
	mProjectInitialized{ false },
	mWizardState{ ProjectWizardState::Idle },
	mTempSetupProject{},
	mBackgroundTextureID{"VoidBGID"},
	mSelectedGameObject{nullptr}
{
	mWindow.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(mWindow);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

Application::~Application()
{
	ImGui::SFML::Shutdown();
}

void Application::Run()
{
	mTickClock.restart();
	while (mWindow.isOpen())
	{
		while (const std::optional event = mWindow.pollEvent())
		{
			if (event->is<sf::Event::Closed>()) {
				mWindow.close();
			}
			ImGui::SFML::ProcessEvent(mWindow, *event);
		}
		sf::Time deltaTime = mTickClock.restart();
		Tick(deltaTime);
		Render();
	}
}

void Application::Tick(sf::Time deltaTime)
{
	RenderUI(deltaTime);

	if (mIsFirstFrame) mIsFirstFrame = false;
}

void Application::Render()
{
	mLevelCanvas.setView(mLevelView);

	RenderScene();

	mWindow.clear(sf::Color(30, 30, 30));
	ImGui::SFML::Render(mWindow);
	mWindow.display();
}

void Application::RenderScene()
{
	mLevelCanvas.clear(sf::Color(50, 50, 50));
	const sf::Texture* backgroundTexture = AssetManager::Get().GetTexture(mBackgroundTextureID);
	if (backgroundTexture)
	{
		sf::Sprite backgroundSprite(*backgroundTexture);
		mLevelCanvas.draw(backgroundSprite);
	}
	for (const unique<GameObject>& gameObject : mProject.level.gameObjects)
	{
		mLevelCanvas.draw(gameObject->sprite);
	}

	mLevelCanvas.display();
}

void Application::RenderUI(sf::Time deltaTime)
{
	ImGui::SFML::Update(mWindow, deltaTime);

	if (!mProjectInitialized)
	{
		RenderWizardUI();
	}

	RenderEditorUI();
}

void Application::SetupDefaultDockingLayout(ImGuiID nodeID)
{

	ImGui::DockBuilderRemoveNode(nodeID);
	ImGui::DockBuilderAddNode(nodeID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetMainViewport()->Size);

	ImGuiID dock_main_id = nodeID;
	ImGuiID dock_bottom_id;
	ImGuiID dock_right_id;
	ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, &dock_bottom_id, &dock_main_id);
	ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, &dock_right_id, &dock_main_id);

	ImGui::DockBuilderDockWindow("Asset Library", dock_bottom_id);
	ImGui::DockBuilderDockWindow("Properties", dock_right_id);
	ImGui::DockBuilderDockWindow("Level Viewport", dock_main_id);

	ImGui::DockBuilderFinish(nodeID);
 }

void Application::RenderEditorUI()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Host", nullptr, window_flags);

	ImGui::PopStyleVar(3);

	ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");

	if (mIsFirstFrame) SetupDefaultDockingLayout(dockspaceID);
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f));

	ImGui::End();

	ImGui::Begin("Level Viewport");

	RenderLevelCanvasUI();

	ImGui::End();

	ImGui::Begin("Properties");

	RenderPropertiesUI();

	ImGui::End();

	ImGui::Begin("Asset Library");

	RenderAssetLibraryUI();

	ImGui::End();
}

void Application::RenderWizardUI()
{

	switch (mWizardState)
	{
	case ProjectWizardState::Idle:
		mWizardState = ProjectWizardState::Selection;
		break;
	case ProjectWizardState::Selection:
		ImGui::SetNextWindowSize(ImVec2(200 + ImGui::GetStyle().ItemSpacing.x * 2, 140), ImGuiCond_Always);
		break;
	case ProjectWizardState::CreateProject:
		ImGui::SetNextWindowSize(ImVec2(800, 445), ImGuiCond_Always);
		break;
	}
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::OpenPopup("Create or Load Project");

	if (ImGui::BeginPopupModal("Create or Load Project", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		switch (mWizardState)
		{
		case ProjectWizardState::Selection:
			if (ImGui::Button("New Project", { 200, 50 })) {
				mWizardState = ProjectWizardState::CreateProject; // Cambia stato, non popup!
			}
			if (ImGui::Button("Load Project", { 200, 50 })) {}
			break;

		case ProjectWizardState::CreateProject:
			RenderCreateProject();
			break;
		}
		ImGui::EndPopup();
	}
}

void Application::RenderCreateProject()
{
	ImGui::Text("Level Texture:");
	float buttonWidth = ImGui::CalcTextSize("Browse Files").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::InputTextWithHint("##background_path", "Texture Path", &mTempSetupProject.backgroundTexturePath, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Browse Files"))
		{
			ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseBGFileDlgKey", "Choose Level Texture", "Image Files{.png,.jpg,.jpeg,.PNG,.JPG,.JPEG}", config);
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseBGFileDlgKey", ImGuiWindowFlags_Modal))
		{
			// Se l'utente ha premuto "Ok"
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				// Ottieni il percorso del file selezionato
				mTempSetupProject.backgroundTexturePath = ImGuiFileDialog::Instance()->GetFilePathName();
			}

			// Chiudi il dialog
			ImGuiFileDialog::Instance()->Close();
		}
	}
	ImGui::Separator();
	ImGui::Text("Level Hitboxes:");
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::InputTextWithHint("##hitbox_path", "Texture Path", &mTempSetupProject.hitboxTexturePath, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Browse Files##Hitbox"))
		{
			ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseHBFileDlgKey", "Choose Hitbox Texture Map", ".png{.png,.PNG}", config);
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseHBFileDlgKey", ImGuiWindowFlags_Modal))
		{
			// Se l'utente ha premuto "Ok"
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				// Ottieni il percorso del file selezionato
				mTempSetupProject.hitboxTexturePath = ImGuiFileDialog::Instance()->GetFilePathName();
			}

			// Chiudi il dialog
			ImGuiFileDialog::Instance()->Close();
		}
	}
	{
		ImGui::Checkbox("Enable Hitbox Creation (Black and White PNG)     ", &mTempSetupProject.bHitboxMap);
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding(); // Allinea il testo verticalmente con lo slider
		ImGui::Text("Hitbox Simplification:");
		ImGui::SameLine();
		ImGui::PushItemWidth(147.7f); // Diamo una larghezza fissa allo slider
		ImGui::SliderInt("##hitbox_simplification", &mTempSetupProject.simplifyIndex, 0, 30);
		ImGui::PopItemWidth();
	}
	ImGui::Separator();
	ImGui::Text("Assets:");
	if (ImGui::BeginChild("AssetList", ImVec2(0, 250), ImGuiChildFlags_Borders))
	{
		int asset_to_delete = -1;
		for (int i = 0; i < mTempAssetList.size(); ++i)
		{
			ImGui::PushID(i);
			ImGui::PushItemWidth(210);
			ImGui::InputTextWithHint("##AssetName", "Object Name (as in game file)", &mTempAssetList[i].name);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(415);
			ImGui::InputTextWithHint("##AssetPath", "Sprite Path", &mTempAssetList[i].texturePath, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Browse Files"))
			{
				std::string dialogKey = "ChooseAssetTexKey_" + std::to_string(i);
				ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
				IGFD::FileDialogConfig config;
				config.path = ".";
				config.flags = ImGuiFileDialogFlags_Modal;
				ImGuiFileDialog::Instance()->OpenDialog("ChooseAssetTexKey_" + std::to_string(i), "Choose Asset Texture", "Image Files{.png,.jpg,.jpeg,.PNG,.JPG,.JPEG}", config);
			}
			if (ImGuiFileDialog::Instance()->Display("ChooseAssetTexKey_" + std::to_string(i)))
			{
				// Se l'utente ha premuto "Ok"
				if (ImGuiFileDialog::Instance()->IsOk())
				{
					// Ottieni il percorso del file selezionato
					mTempAssetList[i].texturePath = ImGuiFileDialog::Instance()->GetFilePathName();
				}

				// Chiudi il dialog
				ImGuiFileDialog::Instance()->Close();
			}

			ImGui::SameLine();

			if (ImGui::Button("X")) {
				asset_to_delete = i;
			}

			ImGui::PopID();
		}

		if (asset_to_delete != -1) {
			mTempAssetList.erase(mTempAssetList.begin() + asset_to_delete);
		}

		if (ImGui::Button("Add Asset"))
		{
			mTempAssetList.emplace_back();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();
	if (ImGui::Button("Create Project")) {
		mProjectInitialized = true;
		mWizardState = ProjectWizardState::Idle;
		for (const AssetData assetData : mTempAssetList) {
			if (assetData.name.empty()) continue;
			mTempSetupProject.assets[assetData.name] = assetData;
		}
		mProject = std::move(mTempSetupProject);
		LoadProjectTextures();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Back")) {
		mWizardState = ProjectWizardState::Selection; // Torna allo stato precedente
	}
}

void Application::RenderLevelCanvasUI()
{
	sf::Vector2u newCanvasSize = ImGui::GetContentRegionAvail();

	if (mLevelCanvas.getSize() != newCanvasSize)
	{
		mLevelCanvas.resize(newCanvasSize);
	}

	sf::Vector2f viewSize = mLevelView.getSize();
	float windowRatio = (float)newCanvasSize.x / (float)newCanvasSize.y;
	float viewRatio = viewSize.x / viewSize.y;

	float sizeX = 1.0f;
	float sizeY = 1.0f;
	float posX = 0.0f;
	float posY = 0.0f;

	if (windowRatio > viewRatio)
	{
		sizeX = viewRatio / windowRatio;
		posX = (1.0f - sizeX) / 2.0f;
	}
	else
	{
		sizeY = windowRatio / viewRatio;
		posY = (1.0f - sizeY) / 2.0f;
	}
	mLevelView.setViewport(sf::FloatRect({ posX, posY }, { sizeX, sizeY }));

	ImGui::Image(mLevelCanvas);

	sf::Vector2i viewportMousePos{ sf::Vector2f(ImGui::GetMousePos()) - sf::Vector2f(ImGui::GetItemRectMin()) };
	sf::Vector2f levelMousePos = mLevelCanvas.mapPixelToCoords(viewportMousePos, mLevelView);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
		{
			const char* nameCharID = (const char*)payload->Data;
			std::string nameID(nameCharID);

			const auto& asset = mProject.assets.at(nameID);

			const sf::Texture* gameObjectTexture = AssetManager::Get().GetTexture(nameID);
			if (gameObjectTexture)
			{
				sf::Sprite gameObjectSprite(*gameObjectTexture);
				GameObject newObject(nameID, gameObjectSprite);
				newObject.sprite.setPosition(levelMousePos);
				newObject.sprite.setScale(asset.defaultScale);
				newObject.sprite.setRotation(asset.defaultRotation);
				newObject.sprite.setOrigin(sf::Vector2f{ gameObjectTexture->getSize().x / 2.f, gameObjectTexture->getSize().y / 2.f });
				mProject.level.gameObjects.push_back(std::make_unique<GameObject>(newObject));
			}
		}

		ImGui::EndDragDropTarget();
	}
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		List<unique<GameObject>>& gameObjects = mProject.level.gameObjects;
		for (auto object = gameObjects.rbegin(); object!= gameObjects.rend(); ++object)
		{
			if (object->get()->sprite.getGlobalBounds().contains(levelMousePos))
			{
				mSelectedGameObject = object->get();
				mSelectedAssetID.reset();
				std::rotate((object+1).base(), object.base(), gameObjects.end());
			}
		}
	}

}

void Application::RenderPropertiesUI()
{
	if (mSelectedGameObject)
	{
		ImGui::Text("Position");
		sf::Vector2f position = mSelectedGameObject->sprite.getPosition();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x/2 - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##pos_x_input", &position.x)) {
			mSelectedGameObject->sprite.setPosition(position);
		}

		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##pos_y_input", &position.y)) {
			mSelectedGameObject->sprite.setPosition(position);
		}
		ImGui::PopItemWidth();
		ImGui::Separator();

		
	}
	else if (mSelectedAssetID.has_value())
	{

	}
}

void Application::RenderAssetLibraryUI()
{
	Map<std::string, Asset> assets = mProject.assets;

	ImGuiStyle& style = ImGui::GetStyle();
	float windowVisible_x2 = ImGui::GetWindowPos().x + ImGui::GetContentRegionAvail().x;
	float thumbnailSize = 80.0f;
	float padding = style.ItemSpacing.x;

	for (const auto& pair : assets)
	{
		ImGui::PushID(pair.first.c_str());
		ImGui::BeginGroup();
		{
			if (ImGui::ImageButton("##Asset", *AssetManager::Get().GetTexture(pair.first), sf::Vector2f{ thumbnailSize, thumbnailSize }))
			{
				mSelectedAssetID = pair.first;
				mSelectedGameObject = nullptr;
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ASSET_PAYLOAD", pair.first.c_str(), pair.first.length() + 1);
				ImGui::Image(*AssetManager::Get().GetTexture(pair.first), sf::Vector2f{32, 32});

				ImGui::EndDragDropSource();
			}

			ImGui::TextWrapped("%s", pair.first.c_str());
		}
		ImGui::EndGroup();
		float lastItem_x2 = ImGui::GetItemRectMax().x;
		float nextItem_x1 = lastItem_x2 + padding;
		if (nextItem_x1 + thumbnailSize < windowVisible_x2)
		{
			ImGui::SameLine();
		}
		ImGui::PopID();
	}
}

void Application::LoadProjectTextures()
{
	for (const auto& assetPair : mProject.assets)
	{
		AssetManager::Get().LoadTexture(assetPair.first, assetPair.second.texturePath);
	}
	AssetManager::Get().LoadTexture(mBackgroundTextureID, mProject.backgroundTexturePath);
	sf::Vector2f backgroundSize = sf::Vector2f(AssetManager::Get().GetTexture(mBackgroundTextureID)->getSize());
	mLevelView.setSize(backgroundSize);
	mLevelView.setCenter(backgroundSize / 2.f);
}

