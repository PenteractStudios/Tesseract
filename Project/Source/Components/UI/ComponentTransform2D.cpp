#include "ComponentTransform2D.h"

#include "Globals.h"
#include "GameObject.h"
#include "Components/ComponentBoundingBox2D.h"
#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleTime.h"
#include "Panels/PanelControlEditor.h"

#include "debugdraw.h"
#include "imgui.h"
#include "Math/TransformOps.h"

#include "Utils/Leaks.h"

#define JSON_TAG_POSITION "Position"
#define JSON_TAG_ROTATION "Rotation"
#define JSON_TAG_SCALE "Scale"
#define JSON_TAG_LOCAL_EULER_ANGLES "LocalEulerAngles"
#define JSON_TAG_PIVOT "Pivot"
#define JSON_TAG_PIVOT_POSITION "PivotPosition"
#define JSON_TAG_SIZE "Size"
#define JSON_TAG_ANCHOR_X "AnchorX"
#define JSON_TAG_ANCHOR_Y "AnchorY"

void ComponentTransform2D::Update() {
	CalculateGlobalMatrix();
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
	if (ImGui::DragFloat2("Min (X, Y)", anchMin.ptr(), App->editor->dragSpeed2f, 0, 1)) {
		SetAnchorMin(anchMin);
	}
	if (ImGui::DragFloat2("Max (X, Y)", anchMax.ptr(), App->editor->dragSpeed2f, 0, 1)) {
		SetAnchorMax(anchMax);
	}

	float2 piv = pivot;
	float3 pivPos = pivotPosition;
	ImGui::TextColored(App->editor->titleColor, "Pivot");
	if (ImGui::DragFloat2("Pivot (X, Y)", piv.ptr(), App->editor->dragSpeed2f, -inf, inf)) {
		SetPivot(piv);
	}
	ImGui::InputFloat3("Pivot World Position (X,Y,Z)", pivPos.ptr(), "%.3f", ImGuiInputTextFlags_ReadOnly);

	UpdateUIElements();

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
	jPosition[0] = pivotPosition.x;
	jPosition[1] = pivotPosition.y;
	jPosition[2] = pivotPosition.z;


	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	jSize[0] = size.x;
	jSize[1] = size.y;

	JsonValue jAnchorX = jComponent[JSON_TAG_ANCHOR_X];
	jAnchorX[0] = anchorMin.x;
	jAnchorX[1] = anchorMin.y;

	JsonValue jAnchorY = jComponent[JSON_TAG_ANCHOR_Y];
	jAnchorY[0] = anchorMax.x;
	jAnchorY[1] = anchorMax.y;
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

	JsonValue jAnchorX = jComponent[JSON_TAG_ANCHOR_X];
	anchorMin.Set(jAnchorX[0], jAnchorX[1]);

	JsonValue jAnchorY = jComponent[JSON_TAG_ANCHOR_Y];
	anchorMax.Set(jAnchorY[0], jAnchorY[1]);

	dirty = true;
}

void ComponentTransform2D::DrawGizmos() {
	ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
	float factor = canvasRenderer->GetCanvasScreenFactor();
	if (!App->time->IsGameRunning()) {
		dd::box(GetPosition(), dd::colors::Yellow, size.x * scale.x / 100, size.y * scale.y / 100, 0);
		float3 pivotPosFactor = float3(GetPivotPosition().x / 100, GetPivotPosition().y / 100, GetPivotPosition().z / 100);
		dd::box(pivotPosFactor, dd::colors::OrangeRed, 0.1, 0.1, 0);

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
	// Update the new pivot position
	UpdatePivotPosition();
	InvalidateHierarchy();
}

void ComponentTransform2D::SetPivot(float2 pivot_) {
	pivot = pivot_;
	// Update the new pivot position
	UpdatePivotPosition();
	InvalidateHierarchy();
}

void ComponentTransform2D::UpdatePivotPosition() {
	pivotPosition.x = (size.x * pivot.x - size.x * 0.5) * scale.x + position.x;
	pivotPosition.y = (size.y * pivot.y - size.y * 0.5) * scale.y + position.y;
	InvalidateHierarchy();
}

void ComponentTransform2D::SetSize(float2 size_) {
	size = size_;
	// Update the new pivot position
	UpdatePivotPosition();
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
	// Update the new pivot position
	UpdatePivotPosition();
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

const float4x4 ComponentTransform2D::GetGlobalMatrix() const {
	return globalMatrix;
}

const float4x4 ComponentTransform2D::GetGlobalScaledMatrix(bool scaleFactored, bool view3DActive) const {
	ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();

	float factor = 1.0f;

	if (scaleFactored) {
		if (canvasRenderer) {
			factor = canvasRenderer->GetCanvasScreenFactor();
		}
	}

	if (view3DActive) {
		return globalMatrix * float4x4::Scale(size.x / 100.0f, size.y / 100.0f, 0);
	}
	return globalMatrix * float4x4::Scale(size.x * factor, size.y * factor, 0);
}

void ComponentTransform2D::CalculateGlobalMatrix() {
	bool isPivotMode = App->editor->panelControlEditor.GetRectTool();

	if (dirty) {
		ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
		float factor = canvasRenderer ? canvasRenderer->GetCanvasScreenFactor() : 1;
		localMatrix = float4x4::FromTRS(position * factor, rotation, scale);

		GameObject* parent = GetOwner().GetParent();

		if (parent != nullptr) {
			ComponentTransform2D* parentTransform = parent->GetComponent<ComponentTransform2D>();

			if (parentTransform != nullptr) {
				parentTransform->CalculateGlobalMatrix();
				globalMatrix = parentTransform->globalMatrix * localMatrix;
			} else {
				if (isPivotMode) {
					bool isUsing2D = App->editor->panelScene.IsUsing2D();
					if (isUsing2D) {
						localMatrix = float4x4::FromQuat(rotation, pivotPosition * factor);
						globalMatrix = localMatrix * float4x4::Translate(position * factor) * float4x4::Scale(scale);
					} else {
						localMatrix = float4x4::FromQuat(rotation, pivotPosition / 100);
						globalMatrix = localMatrix * float4x4::Translate(position * factor / 100) * float4x4::Scale(scale);
					}
				} else {
					globalMatrix = localMatrix;
				}
			}
		} else {
			globalMatrix = localMatrix;
		}

		dirty = false;
	}
}

void ComponentTransform2D::UpdateUIElements() {
	if (dirty) { // Means the transform has changed
		ComponentText* text = GetOwner().GetComponent<ComponentText>();
		if (text != nullptr) {
			text->RecalculcateVertices();
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

void ComponentTransform2D::InvalidateHierarchy() {
	Invalidate();

	for (GameObject* child : GetOwner().GetChildren()) {
		ComponentTransform2D* childTransform = child->GetComponent<ComponentTransform2D>();
		if (childTransform != nullptr) {
			childTransform->Invalidate();
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

void ComponentTransform2D::DuplicateComponent(GameObject& owner) {
	ComponentTransform2D* component = owner.CreateComponent<ComponentTransform2D>();
	component->SetPivot(pivot);
	component->SetSize(size);
	component->SetPosition(position);
	component->SetRotation(rotation);
	component->SetScale(scale);
	component->SetAnchorMin(anchorMin);
	component->SetAnchorMax(anchorMax);
	component->dirty = true;
}