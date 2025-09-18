#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include "level/Level.h"
#include "core/Utils.h"
#include "project/Project.h"

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
	
private:
	virtual void Tick(sf::Time deltaTime);
	virtual void Render();
	virtual void RenderUI(sf::Time deltaTime);
	void SetupDefaultDockingLayout(ImGuiID nodeID);
	void RenderScene();
	void RenderEditorUI();
	void RenderAssetLibraryUI();
	void RenderWizardUI();
	void RenderCreateProject();
	void LoadProjectTextures();

	sf::RenderWindow mWindow;
	sf::Clock mTickClock;
	sf::Clock mCleanCycleClock;

	sf::RenderTexture mLevelCanvas;
	sf::View mLevelView;

	Level mLevel;

	float mCleanCycleInterval;
	bool mIsFirstFrame;
	bool mProjectInitialized;
	ProjectWizardState mWizardState;
	std::string mBackgroundPath;
	std::string mHitboxPath;
	bool mEnableHitboxCreation;
	int mHitboxSimplifyValue;
	List<AssetCreationInfo> mCreationTempAssets;
	Project mTempSetupProject;
	Project mProject;
	std::string mBackgroundTextureID;
};