#pragma once

#include "Panel.h"

class PanelAudioMixer : public Panel {
public:
	PanelAudioMixer();

	void Update() override;

	
private:
	float gainMusic = 1.0f;
	float gainFX = 1.0f;
};
