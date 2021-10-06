#include "PanelAudioMixer.h"

#include "Components/ComponentAudioListener.h"
#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleScene.h"
#include "Utils/Pool.h"
#include "Scene.h"

#include "imgui.h"
#include "IconsForkAwesome.h"
#include <string>

#include "Utils/Leaks.h"

PanelAudioMixer::PanelAudioMixer()
	: Panel("Audio Mixer", false) {}

void PanelAudioMixer::Update() {
	ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f), ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FK_MUSIC " ") + name;
	if (ImGui::Begin(windowName.c_str(), &enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		ComponentAudioListener* listener = nullptr;
		if (App->scene->scene->audioListenerComponents.Count() == 0) {
			ImGui::End();
			return;
		}
		Pool<ComponentAudioListener>::Iterator audioListener = App->scene->scene->audioListenerComponents.begin();
		gainMainChannel = (*audioListener).GetAudioVolume();

		ImGui::TextColored(App->editor->titleColor, "Main Volume");
		if (ImGui::SliderFloat("##main_volume", &gainMainChannel, 0.f, 1.f)) {
			(*audioListener).SetAudioVolume(gainMainChannel);
		}
		ImGui::Separator();
		ImGui::NewLine();
		ImGui::TextColored(App->editor->titleColor, "Music Volume");
		if (ImGui::SliderFloat("##music_volume", &gainMusicChannel, 0.f, 1.f)) {
			for (ComponentAudioSource& audioSource : App->scene->scene->audioSourceComponents) {
				if (audioSource.GetIsMusic()) {
					audioSource.SetGainMultiplier(gainMusicChannel);
				}
			}
		}
		ImGui::TextColored(App->editor->titleColor, "SFX Volume");
		if (ImGui::SliderFloat("##sfx_volume", &gainSFXChannel, 0.f, 1.f)) {
			for (ComponentAudioSource& audioSource : App->scene->scene->audioSourceComponents) {
				if (!audioSource.GetIsMusic()) {
					audioSource.SetGainMultiplier(gainSFXChannel);
				}
			}
		}
	}
	ImGui::End();
}

float PanelAudioMixer::GetGainMainChannel() const {
	return gainMainChannel;
}

float PanelAudioMixer::GetGainMusicChannel() const {
	return gainMusicChannel;
}

float PanelAudioMixer::GetGainSFXChannel() const {
	return gainSFXChannel;
}

void PanelAudioMixer::GetGainMainChannel(float _gainMainChannel) {
	gainMainChannel = _gainMainChannel;
}

void PanelAudioMixer::SetGainMusicChannel(float _gainMusicChannel) {
	gainMusicChannel = _gainMusicChannel;
}

void PanelAudioMixer::SetGainSFXChannel(float _gainSFXChannel) {
	gainSFXChannel = _gainSFXChannel;
}
