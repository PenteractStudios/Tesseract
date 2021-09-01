#include "ResourceScene.h"

#include "Application.h"
#include "Scene.h"
#include "Utils/Logging.h"
#include "Modules/ModuleTime.h"

#include "Utils/Leaks.h"

void ResourceScene::FinishLoading() {
	const char* filePath = GetResourceFilePath().c_str();

	// Timer to measure loading a scene
	MSTimer timer;
	timer.Start();
	LOG("Loading scene from path: \"%s\".", filePath);

	// Create scene
	scene = new Scene(10000);
	scene->LoadFromFile(filePath);

	unsigned timeMs = timer.Stop();
	LOG("Scene loaded in %ums.", timeMs);
}

void ResourceScene::Unload() {
	RELEASE(scene);
}

Scene* ResourceScene::GetScene() {
	return scene;
}

Scene* ResourceScene::TransferScene() {
	Scene* currentScene = scene;
	scene = nullptr;
	return currentScene;
}
