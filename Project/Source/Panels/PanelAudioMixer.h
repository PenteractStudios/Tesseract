#pragma once

#include "Panel.h"

class PanelAudioMixer : public Panel {
public:
	PanelAudioMixer();

	void Update() override;

	float GetGainMainChannel() const;
	float GetGainMusicChannel() const;
	float GetGainSFXChannel() const;

	void GetGainMainChannel(float _gainMainChannel);
	void SetGainMusicChannel(float _gainMusicChannel);
	void SetGainSFXChannel(float _gainSFXChannel);

private:
	float gainMainChannel = 1.0f;
	float gainMusicChannel = 1.0f;
	float gainSFXChannel = 1.0f;
};
