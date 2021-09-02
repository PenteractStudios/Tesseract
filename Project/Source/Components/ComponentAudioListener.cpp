#include "ComponentAudioListener.h"

#include "GameObject.h"
#include "Modules/ModuleEditor.h"
#include "Utils/Logging.h"

#include "AL/al.h"

#include "Utils/Leaks.h"

#define JSON_TAG_GAIN "Gain"
#define JSON_TAG_MODEL_INDEX "ModelIndex"
#define JSON_TAG_CLAMPED "Clamped"
#define JSON_TAG_DISTANCE_MODEL "DistanceModel"

void ComponentAudioListener::Init() {
	alListenerf(AL_GAIN, gain);
	UpdateAudioListener();
	UpdateDistanceModel();
}

void ComponentAudioListener::Update() {
	UpdateAudioListener();
}

void ComponentAudioListener::OnEditorUpdate() {
	if (ImGui::Checkbox("Active", &active)) {
		if (GetOwner().IsActive()) {
			if (active) {
				Enable();
			} else {
				Disable();
			}
		}
	}
	ImGui::Separator();

	if (ImGui::Combo("Distance Model", &model, distanceModels, IM_ARRAYSIZE(distanceModels))) {
		UpdateDistanceModel();
	}

	if (ImGui::Checkbox("Clamped", &clamped)) {
		UpdateDistanceModel();
	}

	ImGui::Separator();
}

void ComponentAudioListener::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_GAIN] = gain;
	jComponent[JSON_TAG_MODEL_INDEX] = model;
	JsonValue jDistanceModel = jComponent[JSON_TAG_DISTANCE_MODEL];
	jDistanceModel = (int) distanceModel;
	jComponent[JSON_TAG_CLAMPED] = clamped;
}

void ComponentAudioListener::Load(JsonValue jComponent) {
	gain = jComponent[JSON_TAG_GAIN];
	JsonValue jDistanceModel = jComponent[JSON_TAG_DISTANCE_MODEL];
	distanceModel = (DistanceModel)(int) jDistanceModel;
	model = (int) distanceModel;
	clamped = jComponent[JSON_TAG_CLAMPED];
}

void ComponentAudioListener::OnEnable() {
	Init();
}

void ComponentAudioListener::OnDisable() {
	alListenerf(AL_GAIN, 0.f);
}

void ComponentAudioListener::UpdateAudioListener() {
	// For now, the AudioListener will follow the direction / position of the Camera attached to the GameObject
	ComponentCamera* camera = GetOwner().GetComponent<ComponentCamera>();
	if (camera == nullptr) {
		LOG("Warning: AudioListener has to be attached to a GameObject with a Camera Component");
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_ORIENTATION, 0.0f, 0.0f, 0.0f);
		return;
	}

	Frustum* frustum = camera->GetFrustum();
	float3 position = frustum->Pos();
	float3 front = frustum->Front();
	float3 up = frustum->Up();
	float orientation[6] {
		front[0],
		front[1],
		front[2],
		up[0],
		up[1],
		up[2],
	};
	alListenerfv(AL_POSITION, position.ptr());
	alListenerfv(AL_ORIENTATION, orientation);
}

void ComponentAudioListener::UpdateDistanceModel() {
	distanceModel = static_cast<DistanceModel>(model);
	switch (distanceModel) {
	case DistanceModel::EXPONENT:
		if (clamped) {
			alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		} else {
			alDistanceModel(AL_EXPONENT_DISTANCE);
		}
		break;
	case DistanceModel::INVERSE:
		if (clamped) {
			alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		} else {
			alDistanceModel(AL_INVERSE_DISTANCE);
		}
		break;
	case DistanceModel::LINEAR:
		if (clamped) {
			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		} else {
			alDistanceModel(AL_EXPONENT_DISTANCE);
		}
		break;
	}
}

float ComponentAudioListener::GetAudioVolume() const {
	return gain;
}

void ComponentAudioListener::SetAudioVolume(float volume) {
	gain = volume;
	alListenerf(AL_GAIN, (ALfloat) volume);
	UpdateAudioListener();
}
