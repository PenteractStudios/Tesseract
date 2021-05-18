#pragma once

#include "Panel.h"
#include "imgui.h"

class PlayerController;

class PanelDebug : public Panel {
public:
	PanelDebug();

	void Update() override;

private:
	void DisplayGamepadInfo(int index, PlayerController* playerController);

private:
	ImVec4 disconnectedColor = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Color used for text and information in ImGui
	ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);			   // Color used for text and information in ImGui
};
