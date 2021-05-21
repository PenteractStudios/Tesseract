#include "ResourceScript.h"

#include "Utils/FileDialog.h"

void ResourceScript::Load() {
	name = FileDialog::GetFileName(GetAssetFilePath().c_str());
}

void ResourceScript::Unload() {
	name = "";
}