#pragma once
#include "Resource.h"
class ResourceVideo : public Resource {
public:
	REGISTER_RESOURCE(ResourceVideo, ResourceType::VIDEO);

	void Load() override; // cargar el archivo de recursos
	void Unload() override; // liberar la mem de ese recurso
};
