#include "PanelScene.h"

#include "Globals.h"
#include "GameObject.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentCamera.h"
#include "Application.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleProject.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleScene.h"
#include "Utils/Logging.h"
#include "Resources/ResourcePrefab.h"
#include "Resources/ResourceScene.h"
#include "Resources/ResourceMesh.h"
#include "Panels/PanelControlEditor.h"

#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "IconsFontAwesome5.h"
#include "Math/float3x3.h"
#include "Math/float2.h"
#include "Geometry/OBB.h"
#include "SDL_mouse.h"
#include <algorithm>

#include "Utils/Leaks.h"

constexpr char* shadingMode[7] = {"Shaded", "Wireframe", "Ambient Occlusion", "Normals", "Positions", "Light Tiles (Opaque)", "Light Tiles (Transparent)"};

PanelScene::PanelScene()
	: Panel("Scene", true) {}

void PanelScene::Update() {
	ImGui::SetNextWindowDockID(App->editor->dockMainId, ImGuiCond_FirstUseEver);
	std::string windowName = std::string(ICON_FA_BORDER_ALL " ") + name;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollWithMouse;

	if (ImGui::Begin(windowName.c_str(), &enabled, flags)) {
		ImGuizmo::OPERATION currentGuizmoOperation = App->editor->panelControlEditor.GetImGuizmoOperation();
		ImGuizmo::MODE currentGuizmoMode = App->editor->panelControlEditor.GetImGuizmoMode();
		bool useSnap = App->editor->panelControlEditor.GetImGuizmoUseSnap();
		float snap[3];
		App->editor->panelControlEditor.GetImguizmoSnap(snap);

		if (ImGui::BeginMenuBar()) {
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
			
			if (ImGui::Button(currentShadingMode)) {
				ImGui::OpenPopup("DrawMode");
			}

			if (ImGui::BeginPopup("DrawMode")) {
				ImGui::TextColored(App->editor->titleColor, "Shading Mode");
				ImGui::Separator();
				for (unsigned int i = 0; i < IM_ARRAYSIZE(shadingMode); i++) {
					bool isSelected = (currentShadingMode == shadingMode[i]);
					if (ImGui::Selectable(shadingMode[i])) {
						currentShadingMode = shadingMode[i];
						App->renderer->UpdateShadingMode(currentShadingMode);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				for (unsigned int i = 0; i < App->renderer->lightFrustumStatic.GetNumberOfCascades(); ++i) {
					std::string str = "StaticDepth " + std::to_string(i);
					if (ImGui::Selectable(str.c_str())) {
						currentShadingMode = "StaticDepth";
						App->renderer->UpdateShadingMode(str.c_str());
					}
				}

				for (unsigned int i = 0; i < App->renderer->lightFrustumDynamic.GetNumberOfCascades(); ++i) {
					std::string str = "DynamicDepth " + std::to_string(i);
					if (ImGui::Selectable(str.c_str())) {
						currentShadingMode = "DynamicDepth";
						App->renderer->UpdateShadingMode(str.c_str());
					}
				}

				for (unsigned int i = 0; i < App->renderer->lightFrustumMainEntities.GetNumberOfCascades(); ++i) {
					std::string str = "MainEntitiesDepth " + std::to_string(i);
					if (ImGui::Selectable(str.c_str())) {
						currentShadingMode = "MainEntitiesDepth";
						App->renderer->UpdateShadingMode(str.c_str());
					}
				}

				ImGui::EndPopup();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			if (ImGui::Checkbox("2D", &App->userInterface->view2DInternal)) {
				for (ComponentCanvas& canvas : App->scene->scene->canvasComponents) {
					canvas.Invalidate();
				}

				for (ComponentTransform2D& transform2D : App->scene->scene->transform2DComponents) {
					transform2D.Invalidate();
				};

				for (ComponentText& text : App->scene->scene->textComponents) {
					text.Invalidate();
				};
			};
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			std::string camera = std::string(ICON_FA_VIDEO);
			if (ImGui::Button(camera.c_str())) {
				ImGui::OpenPopup("Camera");
			}
			if (ImGui::BeginPopup("Camera")) {
				Frustum* frustum = App->camera->GetEngineCamera()->GetFrustum();
				vec front = frustum->Front();
				vec up = frustum->Up();
				ImGui::TextColored(App->editor->titleColor, "Frustum");
				ImGui::InputFloat3("Front", front.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);
				ImGui::InputFloat3("Up", up.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);

				float nearPlane = frustum->NearPlaneDistance();
				float farPlane = frustum->FarPlaneDistance();
				if (ImGui::DragFloat("Near Plane", &nearPlane, 0.1f, 0.0f, farPlane, "%.2f")) {
					App->camera->engineCamera.GetFrustum()->SetViewPlaneDistances(nearPlane, farPlane);
				}
				if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, nearPlane, inf, "%.2f")) {
					App->camera->engineCamera.GetFrustum()->SetViewPlaneDistances(nearPlane, farPlane);
				}
				ImGui::EndPopup();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			if (ImGui::Button("Gizmos")) {
				ImGui::OpenPopup("Gizmos");
			}
			if (ImGui::BeginPopup("Gizmos")) {
				ImGui::Text("General");
				ImGui::Checkbox("Enable/Disable All", &App->renderer->drawDebugDraw);
				ImGui::Separator();
				ImGui::Checkbox("Bounding Boxes", &App->renderer->drawAllBoundingBoxes);
				ImGui::Checkbox("Quadtree", &App->renderer->drawQuadtree);
				ImGui::Checkbox("Camera Frustums", &App->renderer->drawCameraFrustums);
				ImGui::Checkbox("Light Gizmos", &App->renderer->drawLightGizmos);
				ImGui::Checkbox("Animation Bones", &App->renderer->drawAllBones);
				ImGui::Checkbox("NavMesh", &App->renderer->drawNavMesh);
				ImGui::Checkbox("Particle Gizmos", &App->renderer->drawParticleGizmos);
				ImGui::Checkbox("Colliders", &App->renderer->drawColliders);

				ImGui::TextColored(App->editor->titleColor, "Static Light Frustum");
				ImGui::Checkbox("Ortographic##static", &App->renderer->drawStaticLightFrustumOrtographicGizmo);
				if (App->renderer->drawStaticLightFrustumOrtographicGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumStatic.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##static_orto";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexStaticOrtographic, i);
					}
					ImGui::RadioButton("All cascades##static_orto", &App->renderer->indexStaticOrtographic, INT_MAX);
				}

				ImGui::Checkbox("Perspective##static", &App->renderer->drawStaticLightFrustumPerspectiveGizmo);
				if (App->renderer->drawStaticLightFrustumPerspectiveGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumStatic.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##static_pers";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexStaticPerspective, i);
					}
					ImGui::RadioButton("All cascades##static_pers", &App->renderer->indexStaticPerspective, INT_MAX);
				}

				ImGui::TextColored(App->editor->titleColor, "Dynamic Light Frustum");
				ImGui::Checkbox("Ortographic##dynamic", &App->renderer->drawDynamicLightFrustumOrtographicGizmo);
				if (App->renderer->drawDynamicLightFrustumOrtographicGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumDynamic.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##dynamic_orto";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexDynamicOrtographic, i);
					}
					ImGui::RadioButton("All cascades##dynamic_orto", &App->renderer->indexDynamicOrtographic, INT_MAX);
				}

				ImGui::Checkbox("Perspective##dynamic", &App->renderer->drawDynamicLightFrustumPerspectiveGizmo);
				if (App->renderer->drawDynamicLightFrustumPerspectiveGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumDynamic.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##dynamic_pers";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexDynamicPerspective, i);
					}
					ImGui::RadioButton("All cascades##dynamic_pers", &App->renderer->indexDynamicPerspective, INT_MAX);
				}

				ImGui::TextColored(App->editor->titleColor, "Main Entities Light Frustum");
				ImGui::Checkbox("Ortographic##me", &App->renderer->drawMainEntitiesLightFrustumOrtographicGizmo);
				if (App->renderer->drawMainEntitiesLightFrustumOrtographicGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumMainEntities.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##me_orto";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexMainEntitiesOrtographic, i);
					}
					ImGui::RadioButton("All cascades##me_orto", &App->renderer->indexMainEntitiesOrtographic, INT_MAX);
				}

				ImGui::Checkbox("Perspective##me", &App->renderer->drawMainEntitiesLightFrustumPerspectiveGizmo);
				if (App->renderer->drawMainEntitiesLightFrustumPerspectiveGizmo) {
					for (unsigned int i = 0; i < App->renderer->lightFrustumMainEntities.GetNumberOfCascades(); ++i) {
						std::string label = "Cascade " + std::to_string(i) + "##dme_pers";
						ImGui::RadioButton(label.c_str(), &App->renderer->indexMainEntitiesPerspective, i);
					}
					ImGui::RadioButton("All cascades##me_pers", &App->renderer->indexMainEntitiesPerspective, INT_MAX);
				}

				ImGui::Separator();
				ImGui::EndPopup();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

			if (ImGui::Button("Stats")) {
				ImGui::OpenPopup("Stats");
			}
			if (ImGui::BeginPopup("Stats")) {
				char fps[10];
				sprintf_s(fps, 10, "%.1f", logger->fpsLog[logger->fpsLogIndex]);
				char ms[10];
				sprintf_s(ms, 10, "%.1f", logger->msLog[logger->fpsLogIndex]);

				int triangles = App->scene->scene->GetTotalTriangles();

				ImGui::TextColored(App->editor->titleColor, "Framerate");
				ImGui::Text("Frames: ");
				ImGui::SameLine();
				ImGui::TextColored(App->editor->textColor, fps);
				ImGui::Text("Milliseconds: ");
				ImGui::SameLine();
				ImGui::TextColored(App->editor->textColor, ms);
				ImGui::Separator();
				ImGui::TextColored(App->editor->titleColor, "Triangles / scene");
				ImGui::Text("Number: ");
				ImGui::SameLine();
				char trianglesChar[10];
				sprintf_s(trianglesChar, 10, "%d", triangles);
				ImGui::TextColored(App->editor->textColor, trianglesChar);

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();
			ImGui::EndMenuBar();
		}
		// Update viewport size
		ImVec2 size = ImGui::GetContentRegionAvail();
		if (App->renderer->GetViewportSize().x != size.x || App->renderer->GetViewportSize().y != size.y) {
			TesseractEvent resizeEvent(TesseractEventType::SCREEN_RESIZED);

			resizeEvent.Set<ViewportResizedStruct>((int) size.x, (int) size.y);
			App->events->AddEvent(resizeEvent);

			framebufferSize = {
				size.x,
				size.y,
			};
			windowsPos.x = ImGui::GetWindowPos().x;
			windowsPos.y = ImGui::GetWindowPos().y;
		}

		ImVec2 framebufferPosition = ImGui::GetWindowPos();
		framebufferPosition.y += (ImGui::GetWindowHeight() - size.y);

		// Draw
		ImGui::Image((void*) App->renderer->outputTexture, size, ImVec2(0, 1), ImVec2(1, 0));

		if (App->camera->IsEngineCameraActive()) {
			// Drag and drop
			if (ImGui::BeginDragDropTarget()) {
				std::string payloadTypePrefab = std::string("_RESOURCE_") + GetResourceTypeName(ResourceType::PREFAB);
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTypePrefab.c_str())) {
					UID prefabId = *(UID*) payload->Data;
					ResourcePrefab* prefab = App->resources->GetResource<ResourcePrefab>(prefabId);
					if (prefab != nullptr) {
						App->scene->BuildPrefab(prefabId, App->scene->scene->root);
					}
				}

				// TODO: "Are you sure?" Popup to avoid losing the current scene
				std::string payloadTypeScene = std::string("_RESOURCE_") + GetResourceTypeName(ResourceType::SCENE);
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadTypeScene.c_str())) {
					UID sceneId = *(UID*) payload->Data;
					ResourceScene* scene = App->resources->GetResource<ResourceScene>(sceneId);
					if (scene != nullptr) {
						scene->BuildScene();
					}
				}
				ImGui::EndDragDropTarget();
			}

			float viewManipulateRight = framebufferPosition.x + framebufferSize.x;
			float viewManipulateTop = framebufferPosition.y;

			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(framebufferPosition.x, framebufferPosition.y, framebufferSize.x, framebufferSize.y);

			Frustum* engineFrustum = App->camera->GetEngineCamera()->GetFrustum();
			float4x4 cameraView = float4x4(engineFrustum->ViewMatrix()).Transposed();
			float4x4 cameraProjection = engineFrustum->ProjectionMatrix().Transposed();

			GameObject* selectedGameObject = App->editor->selectedGameObject;
			if (selectedGameObject && !selectedGameObject->GetComponent<ComponentTransform2D>()) {
				ComponentTransform* transform = selectedGameObject->GetComponent<ComponentTransform>();
				float4x4 globalMatrix = transform->GetGlobalMatrix().Transposed();

				ImGuizmo::Manipulate(cameraView.ptr(), cameraProjection.ptr(), currentGuizmoOperation, currentGuizmoMode, globalMatrix.ptr(), NULL, useSnap ? snap : NULL);

				if (ImGuizmo::IsUsing()) {
					GameObject* parent = selectedGameObject->GetParent();
					float4x4 inverseParentMatrix = float4x4::identity;
					if (parent != nullptr) {
						ComponentTransform* parentTransform = parent->GetComponent<ComponentTransform>();
						inverseParentMatrix = parentTransform->GetGlobalMatrix().Inverted();
					}
					float4x4 localMatrix = inverseParentMatrix * globalMatrix.Transposed();

					float3 translation;
					Quat rotation;
					float3 scale;
					localMatrix.Decompose(translation, rotation, scale);

					switch (currentGuizmoOperation) {
					case ImGuizmo::TRANSLATE:
						transform->SetPosition(translation);
						break;
					case ImGuizmo::ROTATE:
						transform->SetRotation(rotation);
						break;
					case ImGuizmo::SCALE:
						transform->SetScale(scale);
						break;
					}
				}
			}

			float viewManipulateSize = 100;
			ImGuizmo::ViewManipulate(cameraView.ptr(), 4, ImVec2(viewManipulateRight - viewManipulateSize, viewManipulateTop), ImVec2(viewManipulateSize, viewManipulateSize), 0x10101010);
			if (ImGui::IsWindowFocused()) {
				float4x4 newCameraView = cameraView.InverseTransposed();
				App->camera->engineCamera.GetFrustum()->SetFrame(newCameraView.Col(3).xyz(), -newCameraView.Col(2).xyz(), newCameraView.Col(1).xyz());
			}
		}

		// Capture input
		if (ImGui::IsWindowFocused()) {
			if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KeyState::KS_IDLE || App->input->GetKey(SDL_SCANCODE_LALT) != KeyState::KS_IDLE) {
				ImGuizmo::Enable(false);
			} else {
				ImGuizmo::Enable(true);
			}

			ImGui::CaptureKeyboardFromApp(false);
			ImGui::CaptureMouseFromApp(true);
			ImGuiIO& io = ImGui::GetIO();
			mousePosOnScene.x = io.MousePos.x - framebufferPosition.x;
			mousePosOnScene.y = io.MousePos.y - framebufferPosition.y;

			mousePosNormalized.x = -1 + 2 * std::max(-1.0f, std::min((io.MousePos.x - framebufferPosition.x) / (size.x), 1.0f));
			mousePosNormalized.y = 1 - 2 * std::max(-1.0f, std::min((io.MousePos.y - framebufferPosition.y) / (size.y), 1.0f));

			if (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered() && !ImGuizmo::IsOver()) {
				App->camera->CalculateFrustumNearestObject(mousePosNormalized);
			}
			ImGui::CaptureMouseFromApp(false);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

const float2& PanelScene::GetWindowsPos() const {
	return windowsPos;
}

const float2& PanelScene::GetMousePosOnScene() const {
	return mousePosOnScene;
}
const float2& PanelScene::GetMousePosOnSceneNormalized() const {
	return mousePosNormalized;
}

const char* PanelScene::GetCurrentShadingMode() const {
	return currentShadingMode;
}