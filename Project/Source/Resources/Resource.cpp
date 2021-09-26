#include "Resource.h"

#include "Application.h"
#include "Modules/ModuleTime.h"

Resource::Resource(ResourceType type_, UID id_, const char* name_, const char* assetFilePath_, const char* resourceFilePath_)
	: type(type_)
	, id(id_)
	, name(name_)
	, assetFilePath(assetFilePath_)
	, resourceFilePath(resourceFilePath_) {}

Resource::~Resource() {
	if (loaded) Unload();
}

ResourceType Resource::GetType() const {
	return type;
}

UID Resource::GetId() const {
	return id;
}

const std::string& Resource::GetName() const {
	return name;
}

const std::string& Resource::GetAssetFilePath() const {
	return assetFilePath;
}

const std::string& Resource::GetResourceFilePath() const {
	return resourceFilePath;
}

void Resource::SetName(const char* name_) {
	name = name_;
}

bool Resource::IsLoading() {
	return loading;
}

void Resource::SetLoading(bool loading_) {
	loading = loading_;
}

bool Resource::IsLoaded() {
	return loaded;
}

void Resource::SetLoaded(bool loaded_) {
	loaded = loaded_;
}

void Resource::Load() {}

void Resource::FinishLoading() {}

void Resource::Unload() {}

void Resource::LoadResourceMeta(JsonValue jResourceMeta) {}

void Resource::SaveResourceMeta(JsonValue jResourceMeta) {}

void Resource::OnEditorUpdate() {}