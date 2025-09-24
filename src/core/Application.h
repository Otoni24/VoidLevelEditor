#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include "level/Level.h"
#include "core/Utils.h"
#include "project/Project.h"

namespace vle
{
	enum class ProjectWizardState {
		Idle, // Nessuna azione
		Selection,  // Mostra "Nuovo/Carica"
		LoadProject,
		CreateProject   // Mostra la configurazione del nuovo progetto
	};

	struct AssetCreationInfo {
		std::string name;
		std::string texturePath;
	};

	class Application
	{
	public:
		Application();
		~Application();
		void Run();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

	private:
		virtual void Tick(sf::Time deltaTime);
		virtual void Render();
		virtual void RenderUI(sf::Time deltaTime);
		void SetupDefaultDockingLayout(ImGuiID nodeID);
		void RenderScene();
		void RenderEditorUI();
		void RenderMainMenuBarUI();
		void LoadProjectDialog();
		void LoadProject();
		void RenderLevelCanvasUI();
		void RenderPropertiesUI();
		void RenderGameObjectPropertiesUI();
		void RenderAssetPropertiesUI();
		void RenderAssetLibraryUI();
		void RenderGameObjectsUI();
		void RenderWizardUI();
		void RenderCreateProject();
		bool ProjectInitialization();
		void LoadProjectTextures();
		void AssignProjectTextures();

		sf::RenderWindow mWindow;
		sf::Clock mTickClock;
		sf::Clock mCleanCycleClock;

		sf::RenderTexture mLevelCanvas;
		sf::View mLevelView;


		float mCleanCycleInterval;
		bool mIsFirstFrame;
		bool mEditingProject;
		bool mProjectInitialized;
		bool mDraggingObject;
		bool mMissingBackgroundPath;
		bool mMissingHitboxPath;
		bool mShowHitboxes;
		ProjectWizardState mWizardState;
		Project mTempSetupProject;
		List<AssetData> mTempAssetList;
		Project mProject;
		std::string mBackgroundTextureID;
		sf::Vector2f mTempObjectPos;

		GameObject* mSelectedGameObject;
		std::optional<std::string> mSelectedAssetID;
	};
}