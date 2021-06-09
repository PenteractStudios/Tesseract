#pragma once

#include "Component.h"

enum class DistanceModel {
	EXPONENT,
	INVERSE,
	LINEAR
};

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

	TESSERACT_ENGINE_API float GetAudioVolume() const;
	TESSERACT_ENGINE_API void SetAudioVolume(float volume);

private:
	float gain = 1.0f;
	DistanceModel distanceModel = DistanceModel::EXPONENT;

	bool clamped = false;
	int model = 0;

	inline static const char* distanceModels[] {"Exponent", "Inverse", "Linear"};
};