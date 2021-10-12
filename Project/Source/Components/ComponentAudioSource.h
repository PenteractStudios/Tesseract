#pragma once

#include "Component.h"

#include "Math/float3.h"

class ComponentAudioSource : public Component {
public:
	REGISTER_COMPONENT(ComponentAudioSource, ComponentType::AUDIO_SOURCE, true); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	~ComponentAudioSource();

	void Init() override;
	void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void UpdateAudioSource();
	bool UpdateSourceParameters();
	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Stop();
	TESSERACT_ENGINE_API void Pause() const;
	TESSERACT_ENGINE_API bool IsPlaying() const;
	TESSERACT_ENGINE_API bool IsPaused() const;
	TESSERACT_ENGINE_API bool IsStopped() const;

	// --- GETTERS ---

	TESSERACT_ENGINE_API bool GetMute() const;
	TESSERACT_ENGINE_API bool GetLoop() const;
	TESSERACT_ENGINE_API float GetGain() const;
	TESSERACT_ENGINE_API float GetPitch() const;
	TESSERACT_ENGINE_API bool GetPlayOnAwake() const;
	TESSERACT_ENGINE_API int GetSpatialBlend() const;
	TESSERACT_ENGINE_API int GetSourceType() const;
	TESSERACT_ENGINE_API float GetInnerAngle() const;
	TESSERACT_ENGINE_API float GetOuterAngle() const;
	TESSERACT_ENGINE_API float GetOuterGain() const;
	TESSERACT_ENGINE_API float GetRollOffFactor() const;
	TESSERACT_ENGINE_API float GetReferenceDistance() const;
	TESSERACT_ENGINE_API float GetMaxDistance() const;
	TESSERACT_ENGINE_API float GetIsMusic() const;
	float GetGainMultiplier() const;

	// --- SETTERS ---

	TESSERACT_ENGINE_API void SetMute(bool _mute);
	TESSERACT_ENGINE_API void SetLoop(bool _loop);
	TESSERACT_ENGINE_API void SetGain(float _gain);
	TESSERACT_ENGINE_API void SetPitch(float _pitch);
	TESSERACT_ENGINE_API void SetPlayOnAwake(bool _playOnAwake);
	TESSERACT_ENGINE_API void SetSpatialBlend(int _spatialBlend);
	TESSERACT_ENGINE_API void SetSourceType(int _sourceType);
	TESSERACT_ENGINE_API void SetInnerAngle(float _innerAngle);
	TESSERACT_ENGINE_API void SetOuterAngle(float _outerAngle);
	TESSERACT_ENGINE_API void SetOuterGain(float _outerGain);
	TESSERACT_ENGINE_API void SetRollOffFactor(float _rollOffFactor);
	TESSERACT_ENGINE_API void SetReferenceDistance(float _referenceDistance);
	TESSERACT_ENGINE_API void SetMaxDistance(float _maxDistance);
	TESSERACT_ENGINE_API void SetIsMusic(float _isMusic);
	void SetGainMultiplier(float _gainMultiplier);

private:
	bool drawGizmos = true;
	bool isStarted = false;
	float3 position = {0.f, 0.f, 0.f};
	float3 direction = {0.f, 0.f, 0.f};

	unsigned sourceId = 0;
	UID audioClipId = 0;

	bool mute = false;
	bool loop = false;
	float gain = 1.f;
	float pitch = 1.f;
	bool playOnAwake = false;
	bool isMusic = false;

	int spatialBlend = 1;	  // 2D = 0; 3D = 1;
	int sourceType = 0;		  // Omnidirectional = 0; Directional = 1;
	float innerAngle = 90.f;  // Inner angle from directional source where gain parameter will be "gain"
	float outerAngle = 180.f; // Outer angle from directional source where gain parameter will be "outerGain"
	float outerGain = 0.f;	  // Gain applied outside outerAngle. Between inner and outer, OpenAL do an interpolation
	float rollOffFactor = 1.0f;
	float referenceDistance = 10.0f;
	float maxDistance = 30.0f;
	float gainMultiplier = 1.0f;
};
