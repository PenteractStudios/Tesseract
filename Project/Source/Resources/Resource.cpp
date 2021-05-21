#include "Resource.h"

#include "Application.h"
#include "Modules/ModuleTime.h"

Resource::Resource(ResourceType type_, UID id_, const char* assetFilePath_, const char* resourceFilePath_)
	: type(type_)
	, id(id_)
	, assetFilePath(assetFilePath_)
	, resourceFilePath(resourceFilePath_) {}

Resource::~Resource() {}

ResourceType Resource::GetType() const {
	return type;
}

UID Resource::GetId() const {
	return id;
}

const std::string& Resource::GetAssetFilePath() const {
	return assetFilePath;
}

const std::string& Resource::GetResourceFilePath() const {
	return resourceFilePath;
}

void Resource::Load() {}

void Resource::Unload() {}

void Resource::OnEditorUpdate() {}
