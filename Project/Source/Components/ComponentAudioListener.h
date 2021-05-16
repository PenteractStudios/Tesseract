#pragma once

#include "Component.h"

class ComponentAudioListener : public Component {
public:
	REGISTER_COMPONENT(ComponentAudioListener, ComponentType::AUDIO_LISTENER, false); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void OnEnable() override;
	void OnDisable() override;

	void UpdateAudioListener();

private:
	float gain = 1.0f;
};