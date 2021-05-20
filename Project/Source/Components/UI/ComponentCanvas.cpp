#include "ComponentCanvas.h"

#include "Application.h"
#include "GameObject.h"
#include "ComponentCanvasRenderer.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "imgui.h"

#include "debugdraw.h"
#include "Utils/Leaks.h"

void ComponentCanvas::Init() {
}

void ComponentCanvas::Save(JsonValue jComponent) const {
}

void ComponentCanvas::Load(JsonValue jComponent) {
	Invalidate();
}

bool ComponentCanvas::CanBeRemoved() const {
	return !AnyChildHasCanvasRenderer(&GetOwner());
}

void ComponentCanvas::OnEditorUpdate() {
	float2 refSize = screenReferenceSize;

	if (ImGui::InputFloat2("Reference Screen Size", refSize.ptr(), "%.0f")) {
		SetScreenReferenceSize(refSize);
	}
	ImGui::Checkbox("Show 2D Screen & Objects", &draw2dObjects);
}

void ComponentCanvas::DrawGizmos() {
	if (!App->time->IsGameRunning() && !App->userInterface->IsUsing2D() && draw2dObjects) {
		float2 canvasSize = GetSize();
		dd::box(float3(canvasSize.x * 0.5f, canvasSize.y * 0.5, 0.0f), dd::colors::DimGray, canvasSize.x, canvasSize.y, 0);
	}
}

void ComponentCanvas::Invalidate() {
	dirty = true;
}

void ComponentCanvas::SetScreenReferenceSize(float2 screenReferenceSize_) {
	screenReferenceSize = screenReferenceSize_;
	Invalidate();
}

float2 ComponentCanvas::GetScreenReferenceSize() const {
	return screenReferenceSize;
}

float2 ComponentCanvas::GetSize() {
	RecalculateSizeAndScreenFactor();
	return size;
}

float ComponentCanvas::GetScreenFactor() {
	RecalculateSizeAndScreenFactor();
	return screenFactor;
}

bool ComponentCanvas::GetDraw2dObjects() const {
	return draw2dObjects;
}

void ComponentCanvas::RecalculateSizeAndScreenFactor() {
	if (dirty) {
		if (App->userInterface->IsUsing2D()) {
			size = App->renderer->GetViewportSize();
		} else {
			size = GetScreenReferenceSize() * SCENE_SCREEN_FACTOR;
		}
		float2 factor = size.Div(screenReferenceSize);
		screenFactor = factor.x < factor.y ? factor.x : factor.y;
		dirty = false;
	}
}

bool ComponentCanvas::AnyChildHasCanvasRenderer(const GameObject* obj) const {
	bool found = obj->GetComponent<ComponentCanvasRenderer>();

	for (std::vector<GameObject*>::const_iterator it = obj->GetChildren().begin(); it != obj->GetChildren().end() && !found; ++it) {
		found = AnyChildHasCanvasRenderer(*it);
	}
	return found;
}
