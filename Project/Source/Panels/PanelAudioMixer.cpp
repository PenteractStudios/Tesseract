#include "PanelAudioMixer.h"

#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleAudio.h"
#include "Utils/Pool.h"
#include "Scene.h"

#include "imgui.h"
#include "IconsForkAwesome.h"
#include <string>
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

PanelAudioMixer::PanelAudioMixer()
	: Panel("Audio Mixer", false) {}

void PanelAudioMixer::Update() {
	ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f), ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FK_MUSIC " ") + name;
	if (ImGui::Begin(windowName.c_str(), &enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (App->scene->scene->audioListenerComponents.Count() == 0) {
			ImGui::End();
			return;
		}

		float gainMainChannel = App->audio->GetGainMainChannel();
		float gainMusicChannel = App->audio->GetGainMusicChannel();
		float gainSFXChannel = App->audio->GetGainSFXChannel();

		ImGui::TextColored(App->editor->titleColor, "Main Volume");
		if (ImGui::SliderFloat("##main_volume", &gainMainChannel, 0.f, 1.f)) {
			App->audio->SetGainMainChannel(gainMainChannel);
		}
		ImGui::Separator();
		ImGui::NewLine();
		ImGui::TextColored(App->editor->titleColor, "Music Volume");
		if (ImGui::SliderFloat("##music_volume", &gainMusicChannel, 0.f, 1.f)) {
			App->audio->SetGainMusicChannel(gainMusicChannel);
		}
		ImGui::TextColored(App->editor->titleColor, "SFX Volume");
		if (ImGui::SliderFloat("##sfx_volume", &gainSFXChannel, 0.f, 1.f)) {
			App->audio->SetGainSFXChannel(gainSFXChannel);
		}
	}
	ImGui::End();
}
