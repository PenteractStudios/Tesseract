#pragma once

#include "Modules/Module.h"
#include "Utils/UID.h"

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
	void PreloadScene(UID sceneId);
	void ChangeScene(UID sceneId);
	Scene* GetCurrentScene();

	void DestroyGameObjectDeferred(GameObject* gameObject); //Event dependant destruction, Gameobjects are destroyed upon the receival of an event, so that info is not null

public:
	UID startSceneId = 0; // First scene to be loaded when in GAME configuration

	//Temporary hardcoded solution
	bool godModeOn = false;

private:
	Scene* scene = nullptr;
};
