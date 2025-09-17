#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include "level/Level.h"

enum class LevelWizardState {
	Idle, // Nessuna azione
	Selection,  // Mostra "Nuovo/Carica"
	LoadLevel, 
	CreateLevel   // Mostra la configurazione del nuovo progetto
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
	void SetupDefaultDockingLayout(ImGuiID nodeID);
	void RenderScene();
	void RenderEditorUI();
	void RenderLevelPopup();

	sf::RenderWindow mWindow;
	sf::Clock mTickClock;
	sf::Clock mCleanCycleClock;

	sf::RenderTexture mLevelCanvas;
	sf::View mLevelView;

	Level mLevel;

	float mCleanCycleInterval;
	bool mIsFirstFrame;
	bool mProjectInitialized;
	LevelWizardState mWizardState;
	std::string mBackgroundPath;
	std::string mHitboxPath;
	bool mEnableHitboxCreation;
	int mHitboxSimplifyValue;
};