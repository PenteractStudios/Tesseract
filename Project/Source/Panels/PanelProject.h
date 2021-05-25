#pragma once

#include "Panel.h"

#include <string>

struct AssetFolder;

class PanelProject : public Panel {
public:
	PanelProject();

	void Update() override;

private:
	void UpdateFoldersRecursive(const AssetFolder& folder);
	void UpdateAssets();
	void UpdateResources();
};
