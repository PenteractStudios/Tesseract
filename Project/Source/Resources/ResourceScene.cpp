#include "ResourceScene.h"

#include "Application.h"
#include "Scene.h"
#include "Utils/Logging.h"
#include "Modules/ModuleTime.h"

#include "Utils/Leaks.h"

void ResourceScene::LoadScene() {
	sceneMutex.lock();

	if (scene != nullptr) {
		sceneMutex.unlock();
		return;
	}

	const char* filePath = GetResourceFilePath().c_str();

	// Create scene
	scene = new Scene(10000);

	// Timer to measure loading a scene
	MSTimer timer;
	timer.Start();
	LOG("Loading scene from path: \"%s\".", filePath);

	scene->LoadFromFile(filePath);

	unsigned timeMs = timer.Stop();
	LOG("Scene loaded in %ums.", timeMs);

	sceneMutex.unlock();
}

Scene* ResourceScene::GetScene() {
	Scene* currentScene = nullptr;

	if (sceneMutex.try_lock()) {
		currentScene = scene;

		sceneMutex.unlock();
	}

	return currentScene;
}

Scene* ResourceScene::TransferScene() {
	Scene* currentScene = nullptr;

	if (sceneMutex.try_lock()) {
		currentScene = scene;
		scene = nullptr;

		sceneMutex.unlock();
	}

	return currentScene;
}
