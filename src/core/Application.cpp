#include <imgui_internal.h>
#include "misc/cpp/imgui_stdlib.h"
#include <ImGuiFileDialog.h>
#include "core/Application.h"
#include "core/Utils.h"

Application::Application()
	: mWindow{ sf::VideoMode({1000, 680}), "Level Editor", sf::Style::Default },
	mTickClock{},
	mCleanCycleClock{},
	mCleanCycleInterval{},
	mIsFirstFrame{ true },
	mLevelCanvas({ 800, 600 }),
	mLevel{},
	mProjectInitialized{ false },
	mWizardState{ LevelWizardState::Idle },
	mBackgroundPath{ "" },
	mHitboxPath{ "" },
	mEnableHitboxCreation{ false },
	mHitboxSimplifyValue{ 3 }
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
	ImGui::SFML::Update(mWindow, deltaTime);
	

	if (!mProjectInitialized && mWizardState == LevelWizardState::Idle)
	{
		mWizardState = LevelWizardState::Selection;
		ImGui::OpenPopup("Create or Load Level");
	}
	if (mWizardState != LevelWizardState::Idle)
	{
		switch (mWizardState)
		{
		case LevelWizardState::Selection:
			ImGui::SetNextWindowSize(ImVec2(200+ ImGui::GetStyle().ItemSpacing.x*2, 140), ImGuiCond_Always);
			break;
		case LevelWizardState::CreateLevel:
			ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Always);
			break;
		}
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, { 0.5f, 0.5f });
	}

	if (ImGui::BeginPopupModal("Create or Load Level", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		RenderLevelPopup();
		if (ImGuiFileDialog::Instance()->Display("ChooseBGFileDlgKey", ImGuiFileDialogFlags_Modal))
		{
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mBackgroundPath = ImGuiFileDialog::Instance()->GetFilePathName();
			}
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ChooseHBFileDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk()) {
				mHitboxPath = ImGuiFileDialog::Instance()->GetFilePathName();
			}
			ImGuiFileDialog::Instance()->Close();
		}
		ImGui::EndPopup();
	}

	RenderEditorUI();

	if (mIsFirstFrame) mIsFirstFrame = false;
}

void Application::Render()
{
	RenderScene();

	mWindow.clear(sf::Color(30, 30, 30));
	ImGui::SFML::Render(mWindow);
	mWindow.display();
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

void Application::RenderScene()
{
	mLevelCanvas.clear(sf::Color::Blue);
	sf::CircleShape shape(100.f);
	shape.setOrigin({ 100, 100 });
	shape.setPosition({ mLevelCanvas.getSize().x / 2.f, mLevelCanvas.getSize().y / 2.f });
	shape.setFillColor(sf::Color::Red);
	mLevelCanvas.draw(shape);
	mLevelCanvas.display();
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
	ImGui::End();
}

void Application::RenderLevelPopup()
{
	switch (mWizardState)
	{
	case LevelWizardState::Selection:
		if (ImGui::Button("New Level", {200, 50})) {
			mWizardState = LevelWizardState::CreateLevel; // Cambia stato, non popup!
		}
		if (ImGui::Button("Load Level", {200, 50})) {}
		break;

	case LevelWizardState::CreateLevel:
		ImGui::Text("Level Texture:");
		float buttonWidth = ImGui::CalcTextSize("Browse Files").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
			ImGui::InputText("##background_path", &mBackgroundPath, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("Browse Files"))
			{
				// 1. Crea un oggetto di configurazione
				IGFD::FileDialogConfig config;

				// 2. Imposta il path iniziale se vuoi (opzionale)
				config.path = ".";

				// 3. IMPOSTA LA FLAG FONDAMENTALE
				config.flags = ImGuiFileDialogFlags_Modal;

				// 4. Passa la configurazione a OpenDialog
				ImGuiFileDialog::Instance()->OpenDialog("ChooseBGFileDlgKey", "Choose Level Texture", ".png,.jpg,.jpeg", config);
			}
		}
		ImGui::Separator();
		ImGui::Text("Level Hitboxes:");
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
			ImGui::InputText("##hitbox_path", &mHitboxPath, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("Browse Files##Hitbox"))
			{
				ImGuiFileDialog::Instance()->OpenDialog("ChooseHBFileDlgKey", "Choose Hitbox Texture Map", ".png");
			}
		}
		{
			ImGui::Checkbox("Enable Hitbox Creation (Black and White PNG)           ", &mEnableHitboxCreation);
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding(); // Allinea il testo verticalmente con lo slider
			ImGui::Text("Hitbox Simplification:");
			ImGui::SameLine();
			ImGui::PushItemWidth(150.0f); // Diamo una larghezza fissa allo slider
			ImGui::SliderInt("##hitbox_simplification", &mHitboxSimplifyValue, 0, 30);
			ImGui::PopItemWidth();
		}
		ImGui::Separator();
		if (ImGui::Button("Create Level")) {
			// ... Logica di creazione ...
			mProjectInitialized = true;
			mWizardState = LevelWizardState::Idle;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Back")) {
			mWizardState = LevelWizardState::Selection; // Torna allo stato precedente
		}
		break;
	}
}