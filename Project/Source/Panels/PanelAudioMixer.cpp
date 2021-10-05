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
		Pool<ComponentAudioSource>::Iterator audioSources = App->scene->scene->audioSourceComponents.begin();

		float gain = (*audioListener).GetAudioVolume();

		ImGui::TextColored(App->editor->titleColor, "Listener Volume");
		if (ImGui::SliderFloat("##out_volume", &gain, 0.f, 1.f)) {
			(*audioListener).SetAudioVolume(gain);
		}
		ImGui::TextColored(App->editor->titleColor, "Music Volume");
		if (ImGui::SliderFloat("##music_volume", &gainMusic, 0.f, 1.f)) {
			for (audioSources = App->scene->scene->audioSourceComponents.begin(); audioSources != App->scene->scene->audioSourceComponents.end(); ++audioSources) {
				if ((*audioSources).GetIsMusic()) {
					(*audioSources).SetGainMultiplier(gainMusic);
					(*audioSources).UpdateSourceParameters();
				}
			}
		}
		ImGui::TextColored(App->editor->titleColor, "FX Volume");
		if (ImGui::SliderFloat("##fx_volume", &gainFX, 0.f, 1.f)) {
			for (audioSources = App->scene->scene->audioSourceComponents.begin(); audioSources != App->scene->scene->audioSourceComponents.end(); ++audioSources) {
				if (!(*audioSources).GetIsMusic()) {
					(*audioSources).SetGainMultiplier(gainFX);
					(*audioSources).UpdateSourceParameters();
				}
			}
		}
	}
	ImGui::End();
}
