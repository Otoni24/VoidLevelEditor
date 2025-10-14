#include <imgui_internal.h>
#include "misc/cpp/imgui_stdlib.h"
#include <ImGuiFileDialog.h>
#include "core/Application.h"
#include "core/Utils.h"
#include "project/AssetManager.h"
#include "io/ImportExport.h"

using namespace vle;

Application::Application()
	: mWindow{ sf::VideoMode({1000, 680}), "Level Editor", sf::Style::Default },
	mDraggingObject{ false },
	mEditingProject{ false },
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
	mSelectedGameObject{nullptr},
	mMissingBackgroundPath{false},
	mMissingHitboxPath{false},
	mShowHitboxes{false}
{
	mWindow.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(mWindow);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().IniFilename = NULL;
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
		if (gameObject->sprite.has_value())
		{
			mLevelCanvas.draw(*gameObject->sprite);
		}
	}
	if (mSelectedGameObject != nullptr)
	{
		sf::Transform  transform = mSelectedGameObject->sprite->getTransform();
		sf::FloatRect bounds = mSelectedGameObject->sprite->getLocalBounds();
		sf::Vector2f topL = bounds.position;
		sf::Vector2f topR = { topL.x + bounds.size.x, topL.y };
		sf::Vector2f botL = { topL.x, topL.y + bounds.size.y };
		sf::Vector2f botR = topL + bounds.size;
		sf::VertexArray selectionRect(sf::PrimitiveType::LineStrip);
		selectionRect.append(sf::Vertex{ transform.transformPoint(topL), sf::Color::Yellow });
		selectionRect.append(sf::Vertex{ transform.transformPoint(topR), sf::Color::Yellow });
		selectionRect.append(sf::Vertex{ transform.transformPoint(botR), sf::Color::Yellow });
		selectionRect.append(sf::Vertex{ transform.transformPoint(botL), sf::Color::Yellow });
		selectionRect.append(sf::Vertex{ transform.transformPoint(topL), sf::Color::Yellow });
		mLevelCanvas.draw(selectionRect);
	}
	if (mShowHitboxes)
	{
		const sf::Texture* backgroundTexture = AssetManager::Get().GetTexture(mBackgroundTextureID);
		if (backgroundTexture)
		{
			sf::Vector2f backgroundSize = sf::Vector2f(backgroundTexture->getSize());
			sf::RectangleShape hitboxBackground;
			hitboxBackground.setSize(backgroundSize);
			hitboxBackground.setFillColor(sf::Color(10, 10, 10, 200));
			mLevelCanvas.draw(hitboxBackground);
			for (sf::VertexArray chain : mProject.level.hitboxMap)
			{
				mLevelCanvas.draw(chain);
			}
		}
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

	if (ImGuiFileDialog::Instance()->Display("SavePrjFile", ImGuiCond_Always, { 500.f, 300.f }))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			ImportExport::save(mProject, ImGuiFileDialog::Instance()->GetFilePathName());
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("OpenPrjFile", ImGuiCond_Always, { 500.f, 300.f }))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			LoadProject();
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("ExportLevelFile", ImGuiCond_Always, { 500.f, 300.f }))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			List<sf::VertexArray> hitboxMap = mProject.level.hitboxMap;
			if (!mProject.bHitboxCreateLoop)
			{
				for (sf::VertexArray& chain : hitboxMap)
				{
					if (chain.getVertexCount() > 0)
					{
						chain.resize(chain.getVertexCount() - 1);
					}
				}
				mProject.level.hitboxMap = hitboxMap;
			}
			ImportExport::exportLevel(mProject.level, ImGuiFileDialog::Instance()->GetFilePathName());
		}
		ImGuiFileDialog::Instance()->Close();
	}

}

void Application::RenderEditorUI()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
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

	ImGui::BeginMainMenuBar();

	RenderMainMenuBarUI();

	ImGui::EndMainMenuBar();

	ImGui::Begin("Level Viewport");

	RenderLevelCanvasUI();

	ImGui::End();

	ImGui::Begin("Properties");

	RenderPropertiesUI();

	ImGui::End();

	ImGui::Begin("Asset Library");

	RenderAssetLibraryUI();

	ImGui::End();

	ImGui::Begin("Game Objects");

	RenderGameObjectsUI();

	ImGui::End();
}

void Application::SetupDefaultDockingLayout(ImGuiID nodeID)
{

	ImGui::DockBuilderRemoveNode(nodeID);
	ImGui::DockBuilderAddNode(nodeID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetMainViewport()->Size);

	ImGuiID dock_main_id = nodeID;
	ImGuiID dock_bottom_id;
	ImGuiID dock_right_id;
	ImGuiID dock_right_bottom_id;
	ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, &dock_right_id, &dock_main_id);
	ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, &dock_bottom_id, &dock_main_id);
	ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.5f, &dock_right_bottom_id, &dock_right_id);

	ImGui::DockBuilderDockWindow("Asset Library", dock_bottom_id);
	ImGui::DockBuilderDockWindow("Game Objects", dock_right_id);
	ImGui::DockBuilderDockWindow("Level Viewport", dock_main_id);
	ImGui::DockBuilderDockWindow("Properties", dock_right_bottom_id);

	ImGui::DockBuilderFinish(nodeID);
 }

void Application::RenderMainMenuBarUI()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Project"))
		{
			mTempSetupProject = Project();
			mTempAssetList.clear();
			mSelectedAssetID.reset();
			mSelectedGameObject = nullptr;
			mWizardState = ProjectWizardState::CreateProject;
			mProjectInitialized = false;
		}
		if (ImGui::MenuItem("Open Project"))
		{
			LoadProjectDialog();
		}
		
		if (ImGui::MenuItem("Edit Project"))
		{
			mSelectedAssetID.reset();
			mSelectedGameObject = nullptr;
			mTempAssetList.clear();
			mTempSetupProject = std::move(mProject);
			mTempSetupProject.level.hitboxMap.clear();
			for (const auto& pair : mTempSetupProject.assets) {
				if (pair.second) {
					const Asset& asset = *pair.second;
					AssetData assetData{pair.first, asset.texturePath, asset.defaultScale, asset.defaultRotation};
					mTempAssetList.push_back(assetData);
				}
			}
			mWizardState = ProjectWizardState::CreateProject;
			mProjectInitialized = false;
		}
		if (ImGui::MenuItem("Save Project"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;
			ImGuiFileDialog::Instance()->OpenDialog("SavePrjFile", "Save Project File", ".json{.json, .JSON, .Json}", config);
		}

		if (ImGui::MenuItem("Export Level"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;
			ImGuiFileDialog::Instance()->OpenDialog("ExportLevelFile", "Export Level File", ".json{.json, .JSON, .Json}", config);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		ImGui::MenuItem("View Hitboxes", 0, &mShowHitboxes);

		ImGui::EndMenu();
	}
}

void Application::LoadProjectDialog()
{
	IGFD::FileDialogConfig config;
	config.path = ".";
	config.flags = ImGuiFileDialogFlags_Modal;
	ImGuiFileDialog::Instance()->OpenDialog("OpenPrjFile", "Open Project File", ".json{.json, .JSON, .Json}", config);
}

void Application::LoadProject()
{
	mProject = ImportExport::load(ImGuiFileDialog::Instance()->GetFilePathName()).value();
	mSelectedAssetID.reset();
	mSelectedGameObject = nullptr;
	LoadProjectTextures();
	AssignProjectTextures();
	const sf::Texture* backgroundTexture = AssetManager::Get().GetTexture(mBackgroundTextureID);
	if (backgroundTexture)
	{
		sf::Vector2f backgroundSize = sf::Vector2f(backgroundTexture->getSize());
		mLevelView.setSize(backgroundSize);
		mLevelView.setCenter(backgroundSize / 2.f);
	}
	mProjectInitialized = true;
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
		ImGui::SetNextWindowSize(ImVec2(800, 480), ImGuiCond_Always);
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
			if (ImGui::Button("Load Project", { 200, 50 })) {
				LoadProjectDialog();
			}
			break;

		case ProjectWizardState::CreateProject:
			RenderCreateProject();
			break;
		}
		if (ImGuiFileDialog::Instance()->Display("OpenPrjFile", ImGuiCond_Always, { 500.f, 300.f }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				LoadProject();
			}
			ImGuiFileDialog::Instance()->Close();
		}

		ImGui::EndPopup();
	}
}

void Application::RenderCreateProject()
{
	ImGui::Text("Level Name ID:");
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputTextWithHint("##level_name_id", "Level Name ID", &mTempSetupProject.level.levelNameId);
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}
	ImGui::Separator();
	ImGui::Text("Level Texture:");
	float buttonWidth = ImGui::CalcTextSize("Browse Files").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::InputTextWithHint("##background_path", "Texture Path", &mTempSetupProject.backgroundTexturePath, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Browse Files"))
		{
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseBGFileDlgKey", "Choose Level Texture", "Image Files{.png,.jpg,.jpeg,.PNG,.JPG,.JPEG}", config);
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseBGFileDlgKey", ImGuiCond_Always, {500.f, 300.f}))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				mTempSetupProject.backgroundTexturePath = ImGuiFileDialog::Instance()->GetFilePathName();
			}
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
			IGFD::FileDialogConfig config;
			config.path = ".";
			config.flags = ImGuiFileDialogFlags_Modal;
			ImGuiFileDialog::Instance()->OpenDialog("ChooseHBFileDlgKey", "Choose Hitbox Texture Map", ".png{.png,.PNG}", config);
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseHBFileDlgKey", ImGuiCond_Always, { 500.f, 300.f }))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				mTempSetupProject.hitboxTexturePath = ImGuiFileDialog::Instance()->GetFilePathName();
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
	{
		ImGui::Checkbox("Enable Hitbox Creation (B&W PNG)   ", &mTempSetupProject.bHitboxMap);
		ImGui::SameLine();
		ImGui::Checkbox("Create Loop   ", &mTempSetupProject.bHitboxCreateLoop);
		ImGui::AlignTextToFramePadding();
		ImGui::SameLine();
		ImGui::Text("Hitbox Simplification:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
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
		if (ProjectInitialization()) {
			mWizardState = ProjectWizardState::Idle;
			ImGui::CloseCurrentPopup();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Back")) {
		mWizardState = ProjectWizardState::Selection; // Torna allo stato precedente
	}
	ImGui::SameLine();
	if (mMissingBackgroundPath)
	{
		ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		ImGui::TextColored(redColor, "%s", "Missing Background Path");
	}
	ImGui::SameLine();
	if (mMissingHitboxPath)
	{
		ImVec4 redColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		ImGui::TextColored(redColor, "%s", "Missing Hitbox Map Path, input a path or remove the checkbox");
	}
}

bool Application::ProjectInitialization()
{
	if (mTempSetupProject.backgroundTexturePath == "")
	{
		mMissingBackgroundPath = true;
		return false;
	}
	mMissingBackgroundPath = false;
	mTempSetupProject.assets.clear();
	for (const AssetData assetData : mTempAssetList) {
		if (assetData.name.empty()) continue;
		mTempSetupProject.assets[assetData.name] = std::make_unique<Asset>(assetData);
	}
	if (mTempSetupProject.bHitboxMap)
	{
		if (mTempSetupProject.hitboxTexturePath == "")
		{
			mMissingHitboxPath = true;
			return false;
		}
		mMissingHitboxPath = false;
		List<Vectorizer::Math::Chain> chains = Vectorizer::vectorizeImage(mTempSetupProject.hitboxTexturePath, mTempSetupProject.simplifyIndex);
		mTempSetupProject.level.hitboxMap.clear();
		for (Vectorizer::Math::Chain chain : chains)
		{
			sf::VertexArray newChain(sf::PrimitiveType::LineStrip);
			for (Vectorizer::Math::Point point : chain)
			{
				sf::Vertex newPoint{ { point.x, point.y } };
				newChain.append(newPoint);
			}
			mTempSetupProject.level.hitboxMap.push_back(newChain);
		}
	}
	mProjectInitialized = true;
	mProject = std::move(mTempSetupProject);
	LoadProjectTextures();
	for (auto it = mProject.level.gameObjects.begin(); it != mProject.level.gameObjects.end(); )
	{
		GameObject* object = it->get();
		if (mProject.assets.count(object->assetID) == 0)
		{
			it = mProject.level.gameObjects.erase(it);
		}
		else
		{
			const sf::Texture* texture = AssetManager::Get().GetTexture(object->assetID);
			if (texture)
			{
				object->sprite->setTexture(*texture, true);
				object->sprite->setOrigin(sf::Vector2f{ texture->getSize().x / 2.f, texture->getSize().y / 2.f });
			}
			it++;
		}
	}
	const sf::Texture* backgroundTexture = AssetManager::Get().GetTexture(mBackgroundTextureID);
	if (backgroundTexture)
	{
		sf::Vector2f backgroundSize = sf::Vector2f(backgroundTexture->getSize());
		mLevelView.setSize(backgroundSize);
		mLevelView.setCenter(backgroundSize / 2.f);
	}
	return true;
}

void Application::RenderLevelCanvasUI()
{
	ImVec2 newCanvasSize = ImGui::GetContentRegionAvail();

	if (newCanvasSize.x > 0 && newCanvasSize.y > 0)
	{
		sf::Vector2u sfNewCanvasSize = sf::Vector2u(newCanvasSize);
		if (mLevelCanvas.getSize() != sfNewCanvasSize)
		{
			mLevelCanvas.resize(sfNewCanvasSize);

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
		}
	}

	ImGui::Image(mLevelCanvas);

	sf::Vector2i viewportMousePos{ sf::Vector2f(ImGui::GetMousePos()) - sf::Vector2f(ImGui::GetItemRectMin()) };
	sf::Vector2f levelMousePos = mLevelCanvas.mapPixelToCoords(viewportMousePos, mLevelView);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
		{
			const char* nameCharID = (const char*)payload->Data;
			std::string nameID(nameCharID);

			const Asset* asset = mProject.assets.at(nameID).get();

			const sf::Texture* gameObjectTexture = AssetManager::Get().GetTexture(nameID);
			if (gameObjectTexture)
			{
				sf::Sprite gameObjectSprite(*gameObjectTexture);
				GameObject newObject{ nameID, gameObjectSprite };
				newObject.sprite->setPosition(levelMousePos);
				newObject.sprite->setScale(asset->defaultScale);
				newObject.sprite->setRotation(asset->defaultRotation);
				newObject.sprite->setOrigin(sf::Vector2f{ gameObjectTexture->getSize().x / 2.f, gameObjectTexture->getSize().y / 2.f });
				mProject.level.gameObjects.push_back(std::make_unique<GameObject>(newObject));
				mSelectedGameObject = mProject.level.gameObjects.back().get();
			}
		}

		ImGui::EndDragDropTarget();
	}
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			List<unique<GameObject>>& gameObjects = mProject.level.gameObjects;
			for (auto object = gameObjects.rbegin(); object != gameObjects.rend(); ++object)
			{
				if (object->get()->sprite->getGlobalBounds().contains(levelMousePos))
				{
					mSelectedGameObject = object->get();
					mSelectedAssetID.reset();
					mDraggingObject = true;
					break;
				}
				mSelectedGameObject = nullptr;
			}
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			mDraggingObject = false;
		}
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && mDraggingObject)
		{
			ImVec2 mouseDeltaPixels = ImGui::GetIO().MouseDelta;
			sf::Vector2i currentMousePosPixel = sf::Vector2i(ImGui::GetMousePos()) - sf::Vector2i(ImGui::GetItemRectMin());
			sf::Vector2i prevMousePosPixel = currentMousePosPixel - sf::Vector2i(mouseDeltaPixels);
			sf::Vector2f currentMousePosWorld = mLevelCanvas.mapPixelToCoords(currentMousePosPixel, mLevelView);
			sf::Vector2f prevMousePosWorld = mLevelCanvas.mapPixelToCoords(prevMousePosPixel, mLevelView);
			sf::Vector2f worldDelta = currentMousePosWorld - prevMousePosWorld;
			mSelectedGameObject->sprite->move(worldDelta);
		}
	}

}

void Application::RenderPropertiesUI()
{
	if (mSelectedGameObject)
	{
		RenderGameObjectPropertiesUI();
	}
	else if (mSelectedAssetID.has_value())
	{
		RenderAssetPropertiesUI();
	}
}

void Application::RenderGameObjectPropertiesUI()
{
	const float PI = 3.1415926535f;
	{
		ImGui::Text("Position");
		ImGui::Text("x:");
		ImGui::SameLine();
		sf::Vector2f position = mSelectedGameObject->sprite->getPosition();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##pos_x_input", &position.x)) {
			mSelectedGameObject->sprite->setPosition(position);
		}

		ImGui::SameLine();

		ImGui::Text("y:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##pos_y_input", &position.y)) {
			mSelectedGameObject->sprite->setPosition(position);
		}
		ImGui::PopItemWidth();
	}

	ImGui::Separator();

	{
		ImGui::Text("Scale");
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemSpacing.x);
		sf::Vector2f scale = mSelectedGameObject->sprite->getScale();
		if (ImGui::InputFloat("##scale_input", &scale.x)) {
			mSelectedGameObject->sprite->setScale({ scale.x,scale.x });
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::SliderFloat("##scale_slider", &scale.x, 0.f, 10.f)) {
			mSelectedGameObject->sprite->setScale({ scale.x,scale.x });
		}
	}

	ImGui::Separator();

	{
		ImGui::Text("Rotation");
		float rotation = mSelectedGameObject->sprite->getRotation().asDegrees();
		if (rotation > 180.f)
		{
			rotation -= 360.f;
		}
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##rot_input", &rotation)) {
			mSelectedGameObject->sprite->setRotation(sf::degrees(rotation));
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		float rotationRad = mSelectedGameObject->sprite->getRotation().asRadians();
		if (rotationRad > PI)
		{
			rotationRad -= 2 * PI;
		}
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::SliderAngle("##rot_slider", &rotationRad, -180.f, 180.f)) {
			mSelectedGameObject->sprite->setRotation(sf::radians(rotationRad));
		}
		ImGui::PopItemWidth();
	}

	ImGui::Separator();

	{
		auto it = std::find_if(mProject.level.gameObjects.begin(), mProject.level.gameObjects.end(),
			[this](const std::unique_ptr<GameObject>& ptr) {
				return ptr.get() == mSelectedGameObject;
			});
		if (ImGui::Button("Put in Front", { ImGui::GetContentRegionAvail().x , 0 }))
		{
			std::rotate(it, it+1, mProject.level.gameObjects.end());
		}
	}
}

void Application::RenderAssetPropertiesUI()
{
	const float PI = 3.1415926535f;
	Asset* asset = mProject.assets.at(mSelectedAssetID.value()).get();
	{
		ImGui::Text("Scale");
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##scale_input", &asset->defaultScale.x)) {
			asset->defaultScale.y = asset->defaultScale.x;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::SliderFloat("##scale_slider", &asset->defaultScale.x, 0.f, 10.f)) {
			asset->defaultScale.y = asset->defaultScale.x;
		}
		ImGui::PopItemWidth();
		if (ImGui::Button("Apply Scale to Instances", { ImGui::GetContentRegionAvail().x , 0 }))
		{
			for (unique<GameObject>& gameObjectUnique : mProject.level.gameObjects)
			{
				GameObject* gameObject = gameObjectUnique.get();
				if (gameObject->assetID == mSelectedAssetID)
				{
					gameObject->sprite->setScale(asset->defaultScale);
				}
			}
			
		}
	}

	ImGui::Separator();

	{
		ImGui::Text("Rotation");
		float rotation = asset->defaultRotation.asDegrees();
		if (rotation > 180.f)
		{
			rotation -= 360.f;
		}
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputFloat("##rot_input", &rotation)) {
			asset->defaultRotation = sf::degrees(rotation);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		float rotationRad = asset->defaultRotation.asRadians();
		if (rotationRad > PI)
		{
			rotationRad -= 2 * PI;
		}
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::SliderAngle("##rot_slider", &rotationRad, -180.f, 180.f)) {
			asset->defaultRotation = sf::radians(rotationRad);
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::Button("Apply Rotation to Instances", { ImGui::GetContentRegionAvail().x , 0 }))
		{
			for (unique<GameObject>& gameObjectUnique : mProject.level.gameObjects)
			{
				GameObject* gameObject = gameObjectUnique.get();
				if (gameObject->assetID == mSelectedAssetID)
				{
					gameObject->sprite->setRotation(asset->defaultRotation);
				}
			}

		}
		ImGui::PopItemWidth();
	}
}

void Application::RenderAssetLibraryUI()
{
	Map<std::string, unique<Asset>>& assets = mProject.assets;

	ImGuiStyle& style = ImGui::GetStyle();
	float windowVisible_x2 = ImGui::GetWindowPos().x + ImGui::GetContentRegionAvail().x;
	sf::Vector2f thumbnailSize = { 80.0f, 80.f };
	float padding = style.ItemSpacing.x;

	for (const auto& pair : assets)
	{
		const sf::Texture* texture = AssetManager::Get().GetTexture(pair.first);
		if (!texture)
		{
			continue;
		}
		sf::Sprite sprite(*texture);
		ImGui::PushID(pair.first.c_str());
		ImGui::BeginGroup();
		{
			std::string assetName = pair.first;
			ImVec2 thumbnailSize = { 80.0f, 80.0f };
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();

			if (ImGui::Button("##assetButton", thumbnailSize))
			{
				mSelectedAssetID = assetName;
				mSelectedGameObject = nullptr;
			}

			sf::Vector2f textureSize = sf::Vector2f(texture->getSize());
			ImVec2 imageSize = thumbnailSize;
			ImVec2 imagePos = cursorPos;

			float textureRatio = textureSize.x / textureSize.y;
			float widgetRatio = thumbnailSize.x / thumbnailSize.y;

			if (textureRatio > widgetRatio) {
				imageSize.y = thumbnailSize.x / textureRatio;
				imagePos.y += (thumbnailSize.y - imageSize.y) * 0.5f;
			}
			else if (textureRatio < widgetRatio) {
				imageSize.x = thumbnailSize.y * textureRatio;
				imagePos.x += (thumbnailSize.x - imageSize.x) * 0.5f;
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ASSET_PAYLOAD", assetName.c_str(), assetName.length() + 1);
				ImGui::Image(*texture, imageSize);
				ImGui::Text("%s", assetName.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::SetCursorScreenPos({ imagePos.x + 0.1f * thumbnailSize.x, imagePos.y + 0.1f * thumbnailSize.y });
			ImGui::Image(*texture, { imageSize.x * 0.8f, imageSize.y * 0.8f });

			ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + thumbnailSize.y });
			ImGui::TextWrapped("%s", assetName.c_str());
		}
		ImGui::EndGroup();
		float lastItem_x2 = ImGui::GetItemRectMax().x;
		float nextItem_x1 = lastItem_x2 + padding;
		if (nextItem_x1 + thumbnailSize.x < windowVisible_x2)
		{
			ImGui::SameLine();
		}
		ImGui::PopID();
	}
}

void Application::RenderGameObjectsUI()
{
	int index = 0;
	for (auto it = mProject.level.gameObjects.begin(); it != mProject.level.gameObjects.end(); )
	{
		ImGui::PushID(index);
		GameObject* gameObject = it->get();
		float buttonWidth = ImGui::GetContentRegionAvail().x - 20.f;
		bool isSelected = false;
		if (gameObject == mSelectedGameObject)
		{
			isSelected = true;
		}
		std::string label = (gameObject->assetID + "##game_object_list_item");
		if (ImGui::Selectable(label.c_str(), isSelected, 0, { buttonWidth, 0 }))
		{
			mSelectedGameObject = gameObject;
			mSelectedAssetID.reset();
		}
		ImGui::SameLine();
		if (ImGui::Button("X"))
		{
			it = mProject.level.gameObjects.erase(it);
			if (gameObject == mSelectedGameObject) mSelectedGameObject = nullptr;
			++index;
		}
		else {
			++it;
			++index;
		}
		ImGui::PopID();
	}
}

void Application::LoadProjectTextures()
{
	AssetManager::Get().Clear();
	for (const auto& assetPair : mProject.assets)
	{
		AssetManager::Get().LoadTexture(assetPair.first, assetPair.second.get()->texturePath);
	}
	AssetManager::Get().LoadTexture(mBackgroundTextureID, mProject.backgroundTexturePath);
}

void Application::AssignProjectTextures()
{
	for (unique<GameObject>& objectPtr : mProject.level.gameObjects)
	{
		const sf::Texture* texture = AssetManager::Get().GetTexture(objectPtr->assetID);
		objectPtr->sprite.value().setTexture(*texture, true);
	}
}