#include "PanelInspector.h"

#include "Application.h"
#include "Resources/GameObject.h"
#include "Components/Component.h"
#include "Components/ComponentType.h"
#include "Modules/ModuleEditor.h"

#include "Math/float3.h"
#include "Math/float3x3.h"
#include "Math/float4x4.h"

#include "GL/glew.h"
#include "imgui.h"
#include "IconsFontAwesome5.h"
#include "IconsForkAwesome.h"

#include "Utils/Leaks.h"

PanelInspector::PanelInspector()
	: Panel("Inspector", true) {}

void PanelInspector::Update() {
	ImGui::SetNextWindowDockID(App->editor->dockRightId, ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FA_CUBE " ") + name;
	std::string optionsSymbol = std::string(ICON_FK_COG);
	if (ImGui::Begin(windowName.c_str(), &enabled)) {
		GameObject* selected = App->editor->selectedGameObject;
		if (selected != nullptr) {
			ImGui::TextUnformatted("Id:");
			ImGui::SameLine();
			ImGui::TextColored(App->editor->textColor, "%llu", selected->GetID());

			char name[100];
			sprintf_s(name, 100, "%s", selected->name.c_str());
			if (ImGui::InputText("Name", name, 100)) {
				selected->name = name;
			}
			bool active = selected->IsActive();
			if (ImGui::Checkbox("Active##game_object", &active)) {
				if (active) {
					selected->Enable();
				} else {
					selected->Disable();
				}
			}
			// TODO: Fix Me
			ImGui::SameLine();
			HelpMarker("To Fix it.");

			ImGui::Separator();

			// Show Component info
			std::string cName = "";
			for (Component* component : selected->components) {
				switch (component->GetType()) {
				case ComponentType::TRANSFORM:
					cName = "Transformation";
					break;
				case ComponentType::MESH:
					cName = "Mesh Renderer";
					break;
				case ComponentType::CAMERA:
					cName = "Camera";
					break;
				case ComponentType::LIGHT:
					cName = "Light";
					break;
				case ComponentType::BOUNDING_BOX:
					cName = "Bounding Box";
					break;
				default:
					cName = "";
					break;
				}

				ImGui::PushID(component);

				bool headerOpen = ImGui::CollapsingHeader(cName.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);

				// Options BUTTON (in the same line and at the end of the line)
				ImGui::SameLine();
				if (ImGui::GetWindowWidth() > 170) ImGui::Indent(ImGui::GetWindowWidth() - 43);
				if (ImGui::Button(optionsSymbol.c_str())) ImGui::OpenPopup("Component Options");
				// More Component buttons (edit the Indention)...
				if (ImGui::GetWindowWidth() > 170) ImGui::Unindent(ImGui::GetWindowWidth() - 43);

				// Options POPUP
				if (ImGui::BeginPopup("Component Options")) {
					if (component->GetType() != ComponentType::TRANSFORM) {
						if (ImGui::MenuItem("Remove Component")) componentToDelete = component;
					}
					// More Options items ...
					ImGui::EndPopup();
				}

				// Show Component info
				if (headerOpen) component->OnEditorUpdate();

				ImGui::PopID();
			}

			// Add New Components
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			if (ImGui::Button("Add New Component", ImVec2(ImGui::GetContentRegionAvail().x, 25))) { ImGui::OpenPopup("AddComponentPopup"); }
			if (ImGui::BeginPopup("AddComponentPopup")) {
				// Add a Component of type X. If a Component of the same type exists, it wont be created and the modal COMPONENT_EXISTS will show up.
				// Do not include the if() before AddComponent() and the modalToOpen part if the GameObject can have multiple instances of that Component type.
				if (ImGui::MenuItem("Mesh Renderer")) {
					selected->AddComponent(ComponentType::MESH); // TODO: Add more than 1 mesh renderer? or all meshes inside the same component?
				}
				if (ImGui::MenuItem("Camera")) {
					if (!selected->AddComponent(ComponentType::CAMERA)) App->editor->modalToOpen = Modal::COMPONENT_EXISTS;
				}
				if (ImGui::MenuItem("Light")) {
					if (!selected->AddComponent(ComponentType::LIGHT)) App->editor->modalToOpen = Modal::COMPONENT_EXISTS;
				}
				// TRANSFORM is always there, cannot add a new one.
				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}

Component* PanelInspector::GetComponentToDelete() const {
	return componentToDelete;
}

void PanelInspector::SetComponentToDelete(Component* comp) {
	componentToDelete = comp;
}
