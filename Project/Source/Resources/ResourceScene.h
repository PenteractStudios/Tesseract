#pragma once

#include "Resources/Resource.h"
#include <mutex>

class Scene;

class ResourceScene : public Resource {
public:
	REGISTER_RESOURCE(ResourceScene, ResourceType::SCENE);

	void FinishLoading();
	void Unload();

	Scene* GetScene();
	Scene* TransferScene();

private:
	Scene* scene = nullptr;
};
