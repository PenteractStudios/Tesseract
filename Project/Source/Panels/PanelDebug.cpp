#include "PanelDebug.h"

#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleInput.h"

#include "IconsForkAwesome.h"

#include "Utils/Leaks.h"

#define N_PLAYERS 2

PanelDebug::PanelDebug()
	: Panel("Game Controller Debugger", false) {
}

void PanelDebug::Update() {
	ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FK_INFO_CIRCLE " ") + name;
	if (ImGui::Begin(windowName.c_str(), &enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		for (int i = 0; i < N_PLAYERS; ++i) {
			DisplayGamepadInfo(i, App->input->GetPlayerController(i));
			ImGui::Separator();
		}
	}
	ImGui::End();
}

void PanelDebug::DisplayGamepadInfo(int index, PlayerController* playerController) {
	ImGui::TextColored(App->editor->textColor, "Game Controller %d:", index);
	ImGui::SameLine();
	ImGui::TextColored(playerController != nullptr ? yellow : disconnectedColor, playerController != nullptr ? "Connected" : "Disconnected");

	if (playerController != nullptr) {
		//LEFT TRIGGER
		ImGui::TextColored(App->editor->textColor, "Left Trigger:");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_TRIGGERLEFT]) > 0.05f ? yellow : App->editor->textColor, "%.2f",
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_TRIGGERLEFT] / App->input->JOYSTICK_MAX_VALUE);

		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//RIGHT TRIGGER
		ImGui::TextColored(App->editor->textColor, "Right Trigger:");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_TRIGGERRIGHT]) > 0.05f ? yellow : App->editor->textColor, "%.2f", 
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] / App->input->JOYSTICK_MAX_VALUE);

		//LEFT JOYSTICK HORIZONTAL
		ImGui::TextColored(App->editor->textColor, "Left joystick horizontal:");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_LEFTX]) > 0.05f ? yellow : App->editor->textColor, "%.2f",
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_LEFTX] / App->input->JOYSTICK_MAX_VALUE);
		
		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//LEFT JOYSTICK VERTICAL
		ImGui::TextColored(App->editor->textColor, "Left joystick vertical:");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_LEFTY]) > 0.05f ? yellow : App->editor->textColor, "%.2f",
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_LEFTY] / App->input->JOYSTICK_MAX_VALUE);

		//RIGHT JOYSTICK HORIZONTAL
		ImGui::TextColored(App->editor->textColor, "Right joystick horizontal:");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_RIGHTX]) > 0.05f ? yellow : App->editor->textColor, "%.2f",
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_RIGHTX] / App->input->JOYSTICK_MAX_VALUE);

		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//RIGHT JOYSTICK VERTICAL
		ImGui::TextColored(App->editor->textColor, "Right joystick vertical");
		ImGui::SameLine();
		ImGui::TextColored(std::abs(playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_RIGHTY]) > 0.05f ? yellow : App->editor->textColor, "%.2f",
			playerController->gameControllerAxises[SDL_CONTROLLER_AXIS_RIGHTY] / App->input->JOYSTICK_MAX_VALUE);

		//BUTTON A
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_A] ? yellow : App->editor->textColor, "Button A");

		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//BUTTON B
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_B] ? yellow : App->editor->textColor, "Button B");

		//BUTTON X
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_X] ? yellow : App->editor->textColor, "Button X");

		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//BUTTON Y
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_Y] ? yellow : App->editor->textColor, "Button Y");

		//LEFT BUMPER
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] ? yellow : App->editor->textColor, "Left Bumper");

		ImGui::SameLine();
		ImGui::TextColored(App->editor->textColor, "---");
		ImGui::SameLine();

		//RIGHT BUMPER

		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] ? yellow : App->editor->textColor, "Right Bumper");

		ImGui::TextColored(App->editor->textColor, "DPad:");
		ImGui::SameLine();
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT] ? yellow : App->editor->textColor, ICON_FK_ARROW_LEFT);
		ImGui::SameLine();
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] ? yellow : App->editor->textColor, ICON_FK_ARROW_RIGHT);
		ImGui::SameLine();
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_UP] ? yellow : App->editor->textColor, ICON_FK_ARROW_UP);
		ImGui::SameLine();
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_DOWN] ? yellow : App->editor->textColor, ICON_FK_ARROW_DOWN);		
		ImGui::SameLine();
		ImGui::TextColored(playerController->gameControllerButtons[SDL_CONTROLLER_BUTTON_START] ? yellow : App->editor->textColor, ICON_FK_PLAY);


	}
}