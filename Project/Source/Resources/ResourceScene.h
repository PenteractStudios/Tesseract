#pragma once

#include "Resources/Resource.h"
#include <mutex>

class Scene;

class ResourceScene : public Resource {
public:
	REGISTER_RESOURCE(ResourceScene, ResourceType::SCENE);

	void LoadScene();
	Scene* GetScene();
	Scene* TransferScene();

private:
	std::mutex sceneMutex;
	Scene* scene = nullptr;
};
