#include "PanelAbout.h"

#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleHardwareInfo.h"

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

		ImVec2 childSize = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 8);

		ImGui::Checkbox("Libraries", &showLibrary);
		if (showLibrary) {
			ImGui::BeginChildFrame(ImGui::GetID("cfg_infos"), childSize);
			ImGui::Text("Simple DirectMedia Layer (SDL) v%s", App->hardware->sdlVersion);
			ImGui::Text("OpenGL Extension Wrangler Library v%s", App->hardware->glewVersion);
			ImGui::Text("Assimp v%s", App->hardware->assimpVersion);
			ImGui::Text("Dev Image Library (DevIL) v%s", App->hardware->devilVersion);
			ImGui::Text("MathGeoLib v%s", App->hardware->mathgeolibVersion);
			ImGui::Text("ImGui Docking v%s", App->hardware->imguiVersion);
			ImGui::Text("ImGuizmo v%s", App->hardware->imguizmoVersion);
			ImGui::Text("FMT Lib v%s", App->hardware->fmtVersion);
			ImGui::Text("FreeType v%s", App->hardware->freetypeVersion);
			ImGui::Text("Libsndfile v%s", App->hardware->libsndVersion);
			ImGui::Text("PhysicsFS v%s", App->hardware->physFSVersion);
			ImGui::Text("RapidJSON v%s", App->hardware->rapidJsonVersion);
			ImGui::Text("Open Audio Library (OpenAL) v%s", App->hardware->openALVersion);
			ImGui::Text("Bullet Physics v%s", App->hardware->bulletVersion);

			ImGui::EndChildFrame();
		}
	}
	ImGui::End();
}
