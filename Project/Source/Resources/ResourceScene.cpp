#include "ResourceScene.h"

#include "Application.h"
#include "Modules/ModuleScene.h"
#include "FileSystem/SceneImporter.h"

void ResourceScene::BuildScene() {
	App->scene->LoadScene(GetResourceFilePath().c_str());
}