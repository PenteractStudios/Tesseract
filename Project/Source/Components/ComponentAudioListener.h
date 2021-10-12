#pragma once

#include "Component.h"

#include "Math/float3.h"

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
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void OnEnable() override;
	void OnDisable() override;

	void UpdateAudioListener();
	void UpdateDistanceModel();

	void SetAudioVolume(float volume);
	TESSERACT_ENGINE_API void SetPosition(float3 position);
	TESSERACT_ENGINE_API void SetDirection(float3 front, float3 up);

private:
	bool isCustomPos = false;
	
	int model = 2;
	bool clamped = true;
	DistanceModel distanceModel = DistanceModel::LINEAR;
	inline static const char* distanceModels[] {"Exponent", "Inverse", "Linear"};
	float dopplerFactor = 1.0f;
	
};