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
	mLevel{},
	mProjectInitialized{ false },
	mWizardState{ ProjectWizardState::Idle },
	mBackgroundPath{ "" },
	mHitboxPath{ "" },
	mEnableHitboxCreation{ false },
	mHitboxSimplifyValue{ 3 },
	mCreationTempAssets{ },
	mTempSetupProject{},
	mBackgroundTextureID{"VoidBGID"}
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

		sf::Vector2f canvasSize = sf::Vector2f(mLevelCanvas.getSize());
		sf::Vector2f textureSize = sf::Vector2f(backgroundTexture->getSize());

		if (textureSize.x > 0 && textureSize.y > 0)
		{
			float scale = 1.0f;
			sf::Vector2f position(0.f, 0.f);

			float canvasRatio = canvasSize.x / canvasSize.y;
			float textureRatio = textureSize.x / textureSize.y;

			if (canvasRatio > textureRatio)
			{
				scale = canvasSize.y / textureSize.y;
				position.x = (canvasSize.x - textureSize.x * scale) * 0.5f;
			}
			else
			{
				scale = canvasSize.x / textureSize.x;
				position.y = (canvasSize.y - textureSize.y * scale) * 0.5f;
			}
			backgroundSprite.setScale({ scale, scale });
			backgroundSprite.setPosition(position);

			mLevelCanvas.draw(backgroundSprite);
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

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	sf::Vector2u currentCanvasSize = mLevelCanvas.getSize();
	if (currentCanvasSize.x != viewportPanelSize.x || currentCanvasSize.y != viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			mLevelCanvas.resize({ static_cast<unsigned int>(viewportPanelSize.x), static_cast<unsigned int>(viewportPanelSize.y) });
		}
	}

	ImGui::Image(mLevelCanvas);

	ImGui::End();

	ImGui::Begin("Properties");
	ImGui::End();

	ImGui::Begin("Asset Library");

	RenderAssetLibraryUI();

	ImGui::End();
}

void Application::RenderAssetLibraryUI()
{
	List<Asset> assets = mProject.assets;

	ImGuiStyle& style = ImGui::GetStyle();
	float windowVisible_x2 = ImGui::GetWindowPos().x + ImGui::GetContentRegionAvail().x;
	float thumbnailSize = 80.0f;
	float padding = style.ItemSpacing.x;

	for (const Asset asset : assets)
	{
		ImGui::PushID(asset.name.c_str());
		ImGui::BeginGroup();
		sf::Texture ciao = {};
		{
			if (ImGui::ImageButton("##Asset", *AssetManager::Get().GetTexture(asset.name), sf::Vector2f{thumbnailSize, thumbnailSize}))
			{
				LOG("Clicked asset: %s", asset.name.c_str());
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ASSET_PAYLOAD", asset.name.c_str(), asset.name.length() + 1);
				ImGui::Image(*AssetManager::Get().GetTexture(asset.name), sf::Vector2f{32, 32});

				ImGui::EndDragDropSource();
			}

			ImGui::TextWrapped("%s", asset.name.c_str());
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
		for (int i = 0; i < mTempSetupProject.assets.size(); ++i)
		{
			ImGui::PushID(i);
			ImGui::PushItemWidth(210);
			ImGui::InputTextWithHint("##AssetName", "Object Name (as in game file)", &mTempSetupProject.assets[i].name);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(415);
			ImGui::InputTextWithHint("##AssetPath", "Sprite Path", &mTempSetupProject.assets[i].texturePath, ImGuiInputTextFlags_ReadOnly);
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
					mTempSetupProject.assets[i].texturePath = ImGuiFileDialog::Instance()->GetFilePathName();
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
			mTempSetupProject.assets.erase(mTempSetupProject.assets.begin() + asset_to_delete);
		}

		if (ImGui::Button("Add Asset"))
		{
			mTempSetupProject.assets.emplace_back();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();
	if (ImGui::Button("Create Project")) {
		mProjectInitialized = true;
		mWizardState = ProjectWizardState::Idle;
		mProject = std::move(mTempSetupProject);
		LoadProjectTextures();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Back")) {
		mWizardState = ProjectWizardState::Selection; // Torna allo stato precedente
	}
}

void Application::LoadProjectTextures()
{
	for (Asset asset : mProject.assets)
	{
		AssetManager::Get().LoadTexture(asset.name, asset.texturePath);
	}
	AssetManager::Get().LoadTexture(mBackgroundTextureID, mProject.backgroundTexturePath);
}

