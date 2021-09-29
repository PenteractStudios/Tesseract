#pragma once

#include "Resources/Resource.h"

class Scene;

class ResourceScene : public Resource {
public:
	REGISTER_RESOURCE(ResourceScene, ResourceType::SCENE);

	void Load() override;
	void FinishLoading() override;
	void Unload() override;

	Scene* GetScene();
	Scene* TransferScene();

private:
	Scene* scene = nullptr;
};
