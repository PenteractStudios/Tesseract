#include "ComponentTransform2D.h"

#include "Globals.h"
#include "GameObject.h"
#include "Components/ComponentBoundingBox2D.h"
#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleUserInterface.h"
#include "Panels/PanelControlEditor.h"

#include "debugdraw.h"
#include "imgui.h"
#include "Math/TransformOps.h"

#include "Utils/Leaks.h"
#include <Utils/Logging.h>

#define JSON_TAG_POSITION "Position"
#define JSON_TAG_ROTATION "Rotation"
#define JSON_TAG_SCALE "Scale"
#define JSON_TAG_LOCAL_EULER_ANGLES "LocalEulerAngles"
#define JSON_TAG_PIVOT "Pivot"
#define JSON_TAG_PIVOT_POSITION "PivotPosition"
#define JSON_TAG_SIZE "Size"
#define JSON_TAG_ANCHOR_MIN "AnchorMin"
#define JSON_TAG_ANCHOR_MAX "AnchorMax"
#define JSON_TAG_ANCHOR_SELECTED "AnchorSelected"
#define JSON_TAG_IS_CUSTOM_ANCHOR "IsCustomAnchor"

std::array<AnchorPreset, 16> ComponentTransform2D::anchorPresets = {
	AnchorPreset {AnchorPreset::AnchorPresetType::TOP_HORIZONTAL_LEFT_VERTICAL, float2(0.0f, 1.0f), float2(0.0f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::TOP_HORIZONTAL_CENTER_VERTICAL, float2(0.5f, 1.0f), float2(0.5f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::TOP_HORIZONTAL_RIGHT_VERTICAL, float2(1.0f, 1.0f), float2(1.0f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::TOP_HORIZONTAL_STRETCH_VERTICAL, float2(0.0f, 1.0f), float2(1.0f, 1.0f)},

	AnchorPreset {AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_LEFT_VERTICAL, float2(0.0f, 0.5f), float2(0.0f, 0.5f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_CENTER_VERTICAL, float2(0.5f, 0.5f), float2(0.5f, 0.5f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_RIGHT_VERTICAL, float2(1.0f, 0.5f), float2(1.0f, 0.5f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_STRETCH_VERTICAL, float2(0.0f, 0.5f), float2(1.0f, 0.5f)},

	AnchorPreset {AnchorPreset::AnchorPresetType::BOTTOM_HORIZONTAL_LEFT_VERTICAL, float2(0.0f, 0.0f), float2(0.0f, 0.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::BOTTOM_HORIZONTAL_CENTER_VERTICAL, float2(0.5f, 0.0f), float2(0.5f, 0.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::BOTTOM_HORIZONTAL_RIGHT_VERTICAL, float2(1.0f, 0.0f), float2(1.0f, 0.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::BOTTOM_HORIZONTAL_STRETCH_VERTICAL, float2(1.0f, 0.0f), float2(0.0f, 0.0f)},

	AnchorPreset {AnchorPreset::AnchorPresetType::STRETCH_HORIZONTAL_LEFT_VERTICAL, float2(0.0f, 0.0f), float2(0.0f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::STRETCH_HORIZONTAL_CENTER_VERTICAL, float2(0.5f, 0.0f), float2(0.5f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::STRETCH_HORIZONTAL_RIGHT_VERTICAL, float2(1.0f, 0.0f), float2(1.0f, 1.0f)},
	AnchorPreset {AnchorPreset::AnchorPresetType::STRETCH_HORIZONTAL_STRETCH_VERTICAL, float2(0.0f, 0.0f), float2(1.0f, 1.0f)},
};

void ComponentTransform2D::Update() {
}

void ComponentTransform2D::OnEditorUpdate() {
	float3 editorPos = position;

	ImGui::TextColored(App->editor->titleColor, "Transformation (X,Y,Z)");
	if (ImGui::DragFloat3("Position", editorPos.ptr(), App->editor->dragSpeed2f, -inf, inf)) {
		SetPosition(editorPos);
	}

	float3 scl = scale;
	if (ImGui::DragFloat3("Scale", scl.ptr(), App->editor->dragSpeed2f, 0, inf)) {
		SetScale(scl);
	}

	float3 rot = localEulerAngles;
	if (ImGui::DragFloat("Rotation (Z)", &rot.z, App->editor->dragSpeed2f, -inf, inf)) {
		SetRotation(rot);
	}

	float2 editorSize = size;
	ImGui::TextColored(App->editor->titleColor, "Size (Width,Height)");
	if (ImGui::DragFloat2("Size", editorSize.ptr(), App->editor->dragSpeed2f, 0, inf)) {
		SetSize(editorSize);
	}

	float2 anchMin = anchorMin;
	float2 anchMax = anchorMax;
	ImGui::TextColored(App->editor->titleColor, "Anchors");
	if (ImGui::RadioButton("Custom", isCustomAnchor)) isCustomAnchor = true;
	ImGui::SameLine();
	if (ImGui::RadioButton("Presets", !isCustomAnchor)) {
		isCustomAnchor = false;
		anchorSelected = AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_CENTER_VERTICAL;
		SetAnchorMin(anchorPresets[5].anchorMin);
		SetAnchorMax(anchorPresets[5].anchorMax);
	}
	if (isCustomAnchor) { // If we select custom
		anchorSelected = AnchorPreset::AnchorPresetType::CUSTOM;
		if (ImGui::DragFloat2("Min (X, Y)", anchMin.ptr(), App->editor->dragSpeed2f, 0, 1)) {
			SetAnchorMin(anchMin);
		}
		if (ImGui::DragFloat2("Max (X, Y)", anchMax.ptr(), App->editor->dragSpeed2f, 0, 1)) {
			SetAnchorMax(anchMax);
		}
	} else { // If we select presets
		// Anchor Preset Type combo box
		const char* anchorPresetTypeItems[] = {
			"Horizontal: TOP - Vertical: LEFT",
			"Horizontal: TOP - Vertical: CENTER",
			"Horizontal: TOP - Vertical: RIGHT",
			"Horizontal: TOP - Vertical: STRETCH",
			"Horizontal: MIDDLE - Vertical: LEFT",
			"Horizontal: MIDDLE - Vertical: CENTER",
			"Horizontal: MIDDLE - Vertical: RIGHT",
			"Horizontal: MIDDLE - Vertical: STRETCH",
			"Horizontal: BOTTOM - Vertical: LEFT",
			"Horizontal: BOTTOM - Vertical: CENTER",
			"Horizontal: BOTTOM - Vertical: RIGHT",
			"Horizontal: BOTTOM - Vertical: STRETCH",
			"Horizontal: STRETCH - Vertical: LEFT",
			"Horizontal: STRETCH - Vertical: CENTER",
			"Horizontal: STRETCH - Vertical: RIGHT",
			"Horizontal: STRETCH - Vertical: STRETCH",
		};
		const char* anchorPresetCurrent = anchorPresetTypeItems[(int) anchorSelected];
		if (ImGui::BeginCombo("Select Preset", anchorPresetCurrent)) {
			for (int n = 0; n < IM_ARRAYSIZE(anchorPresetTypeItems); ++n) {
				bool isSelected = (anchorPresetCurrent == anchorPresetTypeItems[n]);
				if (ImGui::Selectable(anchorPresetTypeItems[n], isSelected)) {
					anchorSelected = AnchorPreset::AnchorPresetType(n);
					SetAnchorMin(anchorPresets[n].anchorMin);
					SetAnchorMax(anchorPresets[n].anchorMax);
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::InputFloat2("Min (X, Y)", anchMin.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::InputFloat2("Max (X, Y)", anchMax.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);
	}

	float2 piv = pivot;
	float3 pivPos = pivotPosition;
	ImGui::TextColored(App->editor->titleColor, "Pivot");
	if (ImGui::DragFloat2("Pivot (X, Y)", piv.ptr(), App->editor->dragSpeed2f, -inf, inf)) {
		SetPivot(piv);
	}
	ImGui::InputFloat3("Pivot World Position (X,Y,Z)", pivPos.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);

	ImGui::Separator();
}

void ComponentTransform2D::Save(JsonValue jComponent) const {
	JsonValue jPosition = jComponent[JSON_TAG_POSITION];
	jPosition[0] = position.x;
	jPosition[1] = position.y;
	jPosition[2] = position.z;

	JsonValue jRotation = jComponent[JSON_TAG_ROTATION];
	jRotation[0] = rotation.x;
	jRotation[1] = rotation.y;
	jRotation[2] = rotation.z;
	jRotation[3] = rotation.w;

	JsonValue jScale = jComponent[JSON_TAG_SCALE];
	jScale[0] = scale.x;
	jScale[1] = scale.y;
	jScale[2] = scale.z;

	JsonValue jLocalEulerAngles = jComponent[JSON_TAG_LOCAL_EULER_ANGLES];
	jLocalEulerAngles[0] = localEulerAngles.x;
	jLocalEulerAngles[1] = localEulerAngles.y;
	jLocalEulerAngles[2] = localEulerAngles.z;

	JsonValue jPivot = jComponent[JSON_TAG_PIVOT];
	jPivot[0] = pivot.x;
	jPivot[1] = pivot.y;

	JsonValue jPivotPosition = jComponent[JSON_TAG_PIVOT_POSITION];
	jPivotPosition[0] = pivotPosition.x;
	jPivotPosition[1] = pivotPosition.y;
	jPivotPosition[2] = pivotPosition.z;

	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	jSize[0] = size.x;
	jSize[1] = size.y;

	JsonValue jAnchorMin = jComponent[JSON_TAG_ANCHOR_MIN];
	jAnchorMin[0] = anchorMin.x;
	jAnchorMin[1] = anchorMin.y;

	JsonValue jAnchorMax = jComponent[JSON_TAG_ANCHOR_MAX];
	jAnchorMax[0] = anchorMax.x;
	jAnchorMax[1] = anchorMax.y;

	jComponent[JSON_TAG_ANCHOR_SELECTED] = (int) anchorSelected;

	jComponent[JSON_TAG_IS_CUSTOM_ANCHOR] = isCustomAnchor;
}

void ComponentTransform2D::Load(JsonValue jComponent) {
	JsonValue jPosition = jComponent[JSON_TAG_POSITION];
	position.Set(jPosition[0], jPosition[1], jPosition[2]);

	JsonValue jRotation = jComponent[JSON_TAG_ROTATION];
	rotation.Set(jRotation[0], jRotation[1], jRotation[2], jRotation[3]);

	JsonValue jScale = jComponent[JSON_TAG_SCALE];
	scale.Set(jScale[0], jScale[1], jScale[2]);

	JsonValue jLocalEulerAngles = jComponent[JSON_TAG_LOCAL_EULER_ANGLES];
	localEulerAngles.Set(jLocalEulerAngles[0], jLocalEulerAngles[1], jLocalEulerAngles[2]);

	JsonValue jPivot = jComponent[JSON_TAG_PIVOT];
	pivot.Set(jPivot[0], jPivot[1]);

	JsonValue jPivotPosition = jComponent[JSON_TAG_PIVOT_POSITION];
	pivotPosition.Set(jPivotPosition[0], jPivotPosition[1], jPivotPosition[2]);

	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	size.Set(jSize[0], jSize[1]);

	JsonValue jAnchorMin = jComponent[JSON_TAG_ANCHOR_MIN];
	anchorMin.Set(jAnchorMin[0], jAnchorMin[1]);

	JsonValue jAnchorMax = jComponent[JSON_TAG_ANCHOR_MAX];
	anchorMax.Set(jAnchorMax[0], jAnchorMax[1]);

	int posAnchorPresetType = jComponent[JSON_TAG_ANCHOR_SELECTED];
	anchorSelected = (AnchorPreset::AnchorPresetType) posAnchorPresetType;

	isCustomAnchor = jComponent[JSON_TAG_IS_CUSTOM_ANCHOR];

	dirty = true;
}

void ComponentTransform2D::DrawGizmos() {
	ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
	float factor = canvasRenderer ? canvasRenderer->GetCanvasScreenFactor() : 1.0f;
	if (!App->time->IsGameRunning()) {
		//dd::box(GetPosition(), dd::colors::Yellow, size.x * scale.x / 100, size.y * scale.y / 100, 0);
		//dd::box(pivotPosition * factor, dd::colors::OrangeRed, 0.1f, 0.1f, 0.f);
	}
}

bool ComponentTransform2D::CanBeRemoved() const {
	return !HasAnyUIElementsInChildren(&GetOwner());
}

bool ComponentTransform2D::HasAnyUIElementsInChildren(const GameObject* obj) const {
	bool found = obj->GetComponent<ComponentButton>() || obj->GetComponent<ComponentImage>() || obj->GetComponent<ComponentToggle>()
				 || obj->GetComponent<ComponentBoundingBox2D>() || obj->GetComponent<ComponentText>() || obj->GetComponent<ComponentSelectable>()
				 || obj->GetComponent<ComponentCanvasRenderer>() || obj->GetComponent<ComponentCanvas>();

	for (std::vector<GameObject*>::const_iterator it = obj->GetChildren().begin(); it != obj->GetChildren().end() && !found; ++it) {
		found = HasAnyUIElementsInChildren(*it);
	}

	return found;
}

void ComponentTransform2D::SetPosition(float3 position_) {
	position = position_;
	InvalidateHierarchy();
}

void ComponentTransform2D::SetPivot(float2 pivot_) {
	pivot = pivot_;
	CalculatePivotPosition(false);

	InvalidateHierarchy();
}

void ComponentTransform2D::SetSize(float2 size_) {
	size = size_;
	CalculatePivotPosition(false);

	InvalidateHierarchy();
}

void ComponentTransform2D::SetRotation(Quat rotation_) {
	rotation = rotation_;
	localEulerAngles = rotation_.ToEulerXYZ().Mul(RADTODEG);

	InvalidateHierarchy();
}

void ComponentTransform2D::SetRotation(float3 rotation_) {
	rotation = Quat::FromEulerXYZ(rotation_.x * DEGTORAD, rotation_.y * DEGTORAD, rotation_.z * DEGTORAD);
	localEulerAngles = rotation_;

	InvalidateHierarchy();
}

void ComponentTransform2D::SetScale(float3 scale_) {
	scale = scale_;
	CalculatePivotPosition(true);
	InvalidateHierarchy();
}

void ComponentTransform2D::SetAnchorMin(float2 anchorMin_) {
	anchorMin = anchorMin_;
	InvalidateHierarchy();
}

void ComponentTransform2D::SetAnchorMax(float2 anchorMax_) {
	anchorMax = anchorMax_;
	InvalidateHierarchy();
}

const float4x4 ComponentTransform2D::GetGlobalMatrix() {
	CalculateGlobalMatrix();
	return globalMatrix;
}

const float4x4 ComponentTransform2D::GetGlobalScaledMatrix() {
	CalculateGlobalMatrix();
	return globalMatrix * float4x4::Scale(size.x, size.y, 0);
}

float3x3 ComponentTransform2D::GetGlobalRotation() {
	CalculateGlobalMatrix();
	return globalMatrix.RotatePart();
}

float3 ComponentTransform2D::GetGlobalPosition() {
	CalculateGlobalMatrix();
	return globalMatrix.TranslatePart();
}

void ComponentTransform2D::CalculateGlobalMatrix() {
	if (dirty) {
		localMatrix = float4x4::Translate(pivotPosition) * float4x4::FromTRS(GetPositionRelativeToParent(), rotation, scale) * float4x4::Translate(-pivotPosition);

		GameObject* parent = GetOwner().GetParent();

		if (parent != nullptr) {
			ComponentTransform2D* parentTransform = parent->GetComponent<ComponentTransform2D>();

			if (parentTransform != nullptr) {
				parentTransform->CalculateGlobalMatrix();
				globalMatrix = parentTransform->globalMatrix * localMatrix;
			} else {
				globalMatrix = localMatrix;
			}
		} else {
			globalMatrix = localMatrix;
		}

		dirty = false;
		UpdateUIElements();
	}
}

void ComponentTransform2D::UpdateUIElements() {
	if (dirty) { // Means the transform has changed
		ComponentText* text = GetOwner().GetComponent<ComponentText>();
		if (text != nullptr) {
			text->Invalidate();
		}
	}
}

float3 ComponentTransform2D::GetPosition() const {
	return position;
}

float2 ComponentTransform2D::GetSize() const {
	return size;
}

float3 ComponentTransform2D::GetScale() const {
	return scale;
}

float3 ComponentTransform2D::GetPivotPosition() const {
	return pivotPosition;
}



float3 ComponentTransform2D::GetPositionRelativeToParent() const {
	float2 parentSize(0, 0);

	GameObject* parent = GetOwner().GetParent();
	if (parent != nullptr) {
		ComponentCanvas* parentCanvas = parent->GetComponent<ComponentCanvas>();
		ComponentTransform2D* parentTransform2D = parent->GetComponent<ComponentTransform2D>();
		if (parentTransform2D != nullptr) {
			if (parentCanvas != nullptr) {
				parentSize = parentCanvas->GetSize() / parentCanvas->GetScreenFactor();
			} else {
				parentSize = parentTransform2D->GetSize();
			}
		}
	}

	float3 positionRelativeToParent;
	positionRelativeToParent.x = position.x + (parentSize.x * (anchorMin.x - 0.5f));
	positionRelativeToParent.y = position.y + (parentSize.y * (anchorMin.y - 0.5f));
	positionRelativeToParent.z = position.z;
	return positionRelativeToParent;
}

float3 ComponentTransform2D::GetScreenPosition() const {
	float3 screenPosition = GetPositionRelativeToParent();
	GameObject* parent = GetOwner().GetParent();
	while (parent != nullptr) {
		ComponentTransform2D* parentTransform2D = parent->GetComponent<ComponentTransform2D>();
		if (parentTransform2D == nullptr) break;
		screenPosition += parentTransform2D->GetPositionRelativeToParent();
		parent = parent->GetParent();
	}
	return screenPosition;
}

float3 ComponentTransform2D::GetCenterPositionObject() const {
	float4x4 rotateCenterPos = float4x4::FromQuat(rotation, pivotPosition) * float4x4::Translate(GetScreenPosition());
	return float3(rotateCenterPos.x, rotateCenterPos.y, 0.0f);
}

void ComponentTransform2D::CalculatePivotPosition(bool changeScale) {
	pivotPosition.x = size.x * pivot.x - size.x * 0.5f;
	pivotPosition.y = size.y * pivot.y - size.y * 0.5f;
	
	if (changeScale) { // Change position only when the object change the scale
		position.x += (pivotPosition.x * scale.x) - pivotPosition.x;
		position.y += (pivotPosition.y * scale.x) - pivotPosition.y;
	}

	InvalidateHierarchy();
}

void ComponentTransform2D::InvalidateHierarchy() {
	Invalidate();

	for (GameObject* child : GetOwner().GetChildren()) {
		ComponentTransform2D* childTransform = child->GetComponent<ComponentTransform2D>();
		if (childTransform != nullptr) {
			childTransform->InvalidateHierarchy();
		}
	}
}

void ComponentTransform2D::Invalidate() {
	dirty = true;
	ComponentBoundingBox2D* boundingBox = GetOwner().GetComponent<ComponentBoundingBox2D>();
	if (boundingBox != nullptr) {
		boundingBox->Invalidate();
	}
}