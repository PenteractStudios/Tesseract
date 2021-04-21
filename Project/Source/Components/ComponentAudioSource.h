#pragma once

#include "Component.h"
#include "Resources/ResourceAudioClip.h"

#include "AL/al.h"
#include "Math/float3.h"

class ComponentAudioSource : public Component {
public:
	REGISTER_COMPONENT(ComponentAudioSource, ComponentType::AUDIO_SOURCE, true); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	void DuplicateComponent(GameObject& owner) override;

	void UpdateAudioSource();
	void UpdateSourceParameters() const;
	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Stop();
	TESSERACT_ENGINE_API void Pause() const;
	TESSERACT_ENGINE_API bool isPlaying() const;

private:
	bool drawGizmos = true;

	ALuint sourceId = 0;
	float pitch = 1.f;
	float gain = 1.f;
	bool mute = false;
	float3 position = {0.f, 0.f, 0.f};
	float3 direction = {0.f, 0.f, 0.f};
	bool loopSound = false;
	UID audioClipId = 0;
	int spatialBlend = 0;	  // 2D = 0; 3D = 1;
	int sourceType = 0;		  // Omnidirectional = 0; Directional = 1;
	float innerAngle = 90.f;  // Inner angle from directional source where gain parameter will be "gain"
	float outerAngle = 180.f; // Outer angle from directional source where gain parameter will be "outerGain"
	float outerGain = 0.f;	  // Gain applied outside outerAngle. Between inner and outer, OpenAL do an interpolation
};