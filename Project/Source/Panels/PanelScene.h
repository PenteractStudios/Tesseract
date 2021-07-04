#pragma once

#include "Panel.h"

#include "Math/float2.h"

class PanelScene : public Panel {
public:
	PanelScene();

	void Update() override;

	// Getters
	const float2& GetWindowsPos() const;
	const float2& GetMousePosOnScene() const;
	const float2& GetMousePosOnSceneNormalized() const;
	const char* GetCurrentShadingMode() const;

private:
	float2 windowsPos = {0.0f, 0.0f};
	float2 framebufferSize = {0.0f, 0.0f};
	float2 mousePosOnScene = {0.0f, 0.0f};
	float2 mousePosNormalized = {0.0f, 0.0f};
	const char* currentShadingMode = "Shaded";
};
