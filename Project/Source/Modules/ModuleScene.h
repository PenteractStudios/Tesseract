#pragma once

#include "Modules/Module.h"

#include <string>

class Scene;
class GameObject;

class ModuleScene : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;
	bool Start() override;
	UpdateStatus Update() override;
	bool CleanUp() override;
	void ReceiveEvent(TesseractEvent& e) override;

	void CreateEmptyScene(); // Crates a new scene with a default game camera and directional light.

	void DestroyGameObjectDeferred(GameObject* gameObject); //Event dependant destruction, Gameobjects are destroyed upon the receival of an event, so that info is not null

public:
	std::string fileName = ""; // REVIEW. This can be removed? Is it even used for anything?
	Scene* scene = nullptr;
	bool sceneLoaded = false;

	//Temporary hardcoded solution
	bool godModeOn = false;
};
