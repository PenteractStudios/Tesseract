#include "PanelAbout.h"

#include "Application.h"
#include "Modules/ModuleEditor.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

#include "Utils/Leaks.h"

PanelAbout::PanelAbout()
	: Panel("About", false) {}

void PanelAbout::Update() {
	ImGui::SetNextWindowSize(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FK_INFO_CIRCLE " ") + name;
	if (ImGui::Begin(windowName.c_str(), &enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextColored(App->editor->titleColor, "Tesseract Engine");
		ImGui::Separator();
		ImGui::TextColored(App->editor->titleColor, u8"By TBD org.");
		ImGui::Text("Engine developed during the Master in Advanced Programming for AAA Video Games by UPC.");
		ImGui::Text("Engine licensed under the MIT License.");
		ImGui::Separator();

		ImVec2 childSize = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 7);

		ImGui::Checkbox("Libraries", &showLibrary);
		if (showLibrary) {
			ImGui::BeginChildFrame(ImGui::GetID("cfg_infos"), childSize);
			ImGui::Text("MathGeoLib v1.5 ");
			ImGui::Text("Simple DirectMedia Layer (SDL) v2.0");
			ImGui::Text("ImGui Docking");
			ImGui::Text("OpenGL Extension Wrangler Library 2.1.0");
			ImGui::Text("DevIL 1.8.0");
			ImGui::Text("Assimp 1.4.1");
			ImGui::Text("Bullet Physics 2.84");

			ImGui::EndChildFrame();
		}
	}
	ImGui::End();
}
