#pragma once

#include "Resource.h"

class ResourceNavMesh : public Resource {
public:
	REGISTER_RESOURCE(ResourceNavMesh, ResourceType::NAVMESH);

	void Load() override;
	void Unload() override;
};
