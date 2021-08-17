#include "ResourceScene.h"

#include "Application.h"
#include "Importers/SceneImporter.h"

void ResourceScene::BuildScene() {
	SceneImporter::LoadScene(GetResourceFilePath().c_str());
}