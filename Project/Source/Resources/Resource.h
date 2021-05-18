#pragma once

#include "Utils/UID.h"
#include "Resources/ResourceType.h"

#include <string>

class Resource {
public:
	Resource(ResourceType type, UID id, const char* assetFilePath, const char* resourceFilePath);
	virtual ~Resource();

	ResourceType GetType() const;

	UID GetId() const;
	const std::string& GetAssetFilePath() const;
	const std::string& GetResourceFilePath() const;

	virtual void Load();
	virtual void Unload();
	virtual void OnEditorUpdate();

private:
	ResourceType type = ResourceType::UNKNOWN;

	UID id = 0;
	std::string assetFilePath = "";
	std::string resourceFilePath = "";
};
