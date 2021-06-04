#pragma once

#include "Resources/Resource.h"

class Script;

class ResourceScript : public Resource {
public:
	REGISTER_RESOURCE(ResourceScript, ResourceType::SCRIPT);

	void Load() override;
	void Unload() override;
};