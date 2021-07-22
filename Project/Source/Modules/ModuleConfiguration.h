#pragma once

#include "Module.h"

class ModuleConfiguration : public Module {
public:
	bool Init() override;

	void LoadConfiguration();
	void SaveConfiguration();
};
