#include "ComponentAudioListener.h"

#include "GameObject.h"
#include "Application.h"
#include "Modules/ModuleAudio.h"
#include "Modules/ModuleEditor.h"

#include "AL/al.h"

#include "Utils/Leaks.h"

#define JSON_TAG_IS_CUSTOM_POS "IsCustomPos"
#define JSON_TAG_MODEL_INDEX "ModelIndex"
#define JSON_TAG_CLAMPED "Clamped"
#define JSON_TAG_DISTANCE_MODEL "DistanceModel"
#define JSON_TAG_DOPPLER_FACTOR "DopplerFactor"

void ComponentAudioListener::Init() {
	float gain = App->audio->GetGainMainChannel();
	alListenerf(AL_GAIN, gain);
	alDopplerFactor(dopplerFactor);
	if (!isCustomPos) UpdateAudioListener();
	UpdateDistanceModel();
}

void ComponentAudioListener::Update() {
	if (!isCustomPos) UpdateAudioListener();
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
	ImGui::Checkbox("Custom Position", &isCustomPos);
	if (isCustomPos) {
		ImGui::SameLine();
		App->editor->HelpMarker("Set position and direction from script");
	}
	if (ImGui::DragFloat("Doppler factor", &dopplerFactor, App->editor->dragSpeed2f, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
		alDopplerFactor(dopplerFactor);
	}
	if (ImGui::Combo("Distance Model", &model, distanceModels, IM_ARRAYSIZE(distanceModels))) {
		UpdateDistanceModel();
	}

	if (ImGui::Checkbox("Clamped", &clamped)) {
		UpdateDistanceModel();
	}

	ImGui::Separator();
}

void ComponentAudioListener::Load(JsonValue jComponent) {
	isCustomPos = jComponent[JSON_TAG_IS_CUSTOM_POS];
	JsonValue jDistanceModel = jComponent[JSON_TAG_DISTANCE_MODEL];
	distanceModel = (DistanceModel)(int) jDistanceModel;
	model = (int) distanceModel;
	clamped = jComponent[JSON_TAG_CLAMPED];
	dopplerFactor = jComponent[JSON_TAG_DOPPLER_FACTOR];
}

void ComponentAudioListener::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_IS_CUSTOM_POS] = isCustomPos;
	jComponent[JSON_TAG_MODEL_INDEX] = model;
	JsonValue jDistanceModel = jComponent[JSON_TAG_DISTANCE_MODEL];
	jDistanceModel = (int) distanceModel;
	jComponent[JSON_TAG_CLAMPED] = clamped;
	jComponent[JSON_TAG_DOPPLER_FACTOR] = dopplerFactor;
}

void ComponentAudioListener::OnEnable() {
	Init();
}

void ComponentAudioListener::OnDisable() {
	alListenerf(AL_GAIN, 0.f);
}

void ComponentAudioListener::UpdateAudioListener() {
	float3 position;
	float3 front;
	float3 up;

	ComponentCamera* camera = GetOwner().GetComponent<ComponentCamera>();
	if (camera) {
		Frustum* frustum = camera->GetFrustum();
		position = frustum->Pos();
		front = frustum->Front();
		up = frustum->Up();
	} else {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		position = transform->GetGlobalPosition();
		front = transform->GetGlobalRotation() * float3::unitZ;
		up = transform->GetGlobalRotation() * float3::unitY;
	}

	SetPosition(position);
	SetDirection(front, up);
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


void ComponentAudioListener::SetAudioVolume(float volume) {
	alListenerf(AL_GAIN, volume);
}

void ComponentAudioListener::SetPosition(float3 position) {
	alListenerfv(AL_POSITION, position.ptr());
}

void ComponentAudioListener::SetDirection(float3 front, float3 up) {
	float orientation[6] {
		front[0],
		front[1],
		front[2],
		up[0],
		up[1],
		up[2],
	};
	alListenerfv(AL_ORIENTATION, orientation);
}
