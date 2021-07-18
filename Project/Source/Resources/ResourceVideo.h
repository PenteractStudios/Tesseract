#pragma once
#include "Resource.h"
class ResourceVideo : public Resource {
public:
	REGISTER_RESOURCE(ResourceVideo, ResourceType::VIDEO);

	void Load() override;
	void Unload() override;
};
