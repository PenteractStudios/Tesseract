#pragma once

#include "Utils/UID.h"
#include "Resources/ResourceType.h"
#include "FileSystem/JsonValue.h"

#include <string>

class Resource {
public:
	Resource(ResourceType type, UID id, const char* name, const char* assetFilePath, const char* resourceFilePath);
	virtual ~Resource();

	ResourceType GetType() const;

	UID GetId() const;
	const std::string& GetName() const;
	const std::string& GetAssetFilePath() const;
	const std::string& GetResourceFilePath() const;

	void SetName(const char* name);

	virtual void Load();
	virtual void Unload();

	virtual void LoadResourceMeta(JsonValue jResourceMeta);
	virtual void SaveResourceMeta(JsonValue jResourceMeta);
	virtual void OnEditorUpdate();
	virtual void Update();

private:
	ResourceType type = ResourceType::UNKNOWN;

	UID id = 0;
	std::string name = "";
	std::string assetFilePath = "";
	std::string resourceFilePath = "";
};
