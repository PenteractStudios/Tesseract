#include "ComponentTransform.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentBoundingBox.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "PanelHierarchy.h"
#include "PanelInspector.h"
#include "PanelScene.h"

#include "Math/float3x3.h"
#include "SDL.h"

void ComponentTransform::Update()
{
	CalculateGlobalMatrix();
}

void ComponentTransform::OnEditorUpdate()
{
	float3 pos = position;
	float3 scl = scale;
	float3 rot = local_euler_angles;

	if (!App->input->GetMouseButton(SDL_BUTTON_RIGHT))
	{
		if (App->input->GetKey(SDL_SCANCODE_W)) current_guizmo_operation = ImGuizmo::TRANSLATE; // W key
		if (App->input->GetKey(SDL_SCANCODE_E)) current_guizmo_operation = ImGuizmo::ROTATE;
		if (App->input->GetKey(SDL_SCANCODE_R)) current_guizmo_operation = ImGuizmo::SCALE; // R key
	}

	if (ImGui::CollapsingHeader("Transformation"))
	{
		if (ImGui::RadioButton("Translate", current_guizmo_operation == ImGuizmo::TRANSLATE)) current_guizmo_operation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", current_guizmo_operation == ImGuizmo::ROTATE)) current_guizmo_operation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", current_guizmo_operation == ImGuizmo::SCALE)) current_guizmo_operation = ImGuizmo::SCALE;
		ImGui::Separator();

		ImGui::TextColored(title_color, "Transformation (X,Y,Z)");
		if (ImGui::DragFloat3("Position", pos.ptr(), drag_speed2f, -inf, inf))
		{
			SetPosition(pos);
			GetOwner().OnTransformUpdate();
		}
		if (ImGui::DragFloat3("Scale", scl.ptr(), drag_speed2f, 0, inf))
		{
			SetScale(scl);
			GetOwner().OnTransformUpdate();
		}

		if (ImGui::DragFloat3("Rotation", rot.ptr(), drag_speed2f, -inf, inf))
		{
			SetRotation(rot);
			GetOwner().OnTransformUpdate();
			InvalidateHierarchy();
		}

		if (current_guizmo_operation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", current_guizmo_mode == ImGuizmo::LOCAL)) current_guizmo_mode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", current_guizmo_mode == ImGuizmo::WORLD)) current_guizmo_mode = ImGuizmo::WORLD;
		}
		ImGui::Separator();

		ImGui::TextColored(title_color, "Snap");
		ImGui::Checkbox("##snap", &use_snap);
		ImGui::SameLine();

		switch (current_guizmo_operation)
		{
		case ImGuizmo::TRANSLATE:
			ImGui::InputFloat3("Snap", &snap[0]);
			break;
		case ImGuizmo::ROTATE:
			ImGui::InputFloat("Angle Snap", &snap[0]);
			break;
		case ImGuizmo::SCALE:
			ImGui::InputFloat("Scale Snap", &snap[0]);
			break;
		}

		ImGui::Separator();
	}
}

void ComponentTransform::Save(JsonValue& j_component) const
{
	JsonValue& j_position = j_component["Position"];
	j_position[0] = position.x;
	j_position[1] = position.y;
	j_position[2] = position.z;

	JsonValue& j_rotation = j_component["Rotation"];
	j_rotation[0] = rotation.x;
	j_rotation[1] = rotation.y;
	j_rotation[2] = rotation.z;
	j_rotation[3] = rotation.w;

	JsonValue& j_scale = j_component["Scale"];
	j_scale[0] = scale.x;
	j_scale[1] = scale.y;
	j_scale[2] = scale.z;

	JsonValue& j_local_euler_angles = j_component["LocalEulerAngles"];
	j_local_euler_angles[0] = local_euler_angles.x;
	j_local_euler_angles[1] = local_euler_angles.y;
	j_local_euler_angles[2] = local_euler_angles.z;
}

void ComponentTransform::Load(const JsonValue& j_component)
{
	const JsonValue& j_position = j_component["Position"];
	position.Set(j_position[0], j_position[1], j_position[2]);

	const JsonValue& j_rotation = j_component["Rotation"];
	rotation.Set(j_rotation[0], j_rotation[1], j_rotation[2], j_rotation[3]);

	const JsonValue& j_scale = j_component["Scale"];
	scale.Set(j_scale[0], j_scale[1], j_scale[2]);

	const JsonValue& j_local_euler_angles = j_component["LocalEulerAngles"];
	local_euler_angles.Set(j_local_euler_angles[0], j_local_euler_angles[1], j_local_euler_angles[2]);

	dirty = true;
}

void ComponentTransform::InvalidateHierarchy()
{
	Invalidate();

	for (GameObject* child : GetOwner().GetChildren())
	{
		ComponentTransform* child_transform = child->GetComponent<ComponentTransform>();
		if (child_transform != nullptr)
		{
			child_transform->Invalidate();
		}
	}
}

void ComponentTransform::Invalidate()
{
	dirty = true;
	ComponentBoundingBox* bounding_box = GetOwner().GetComponent<ComponentBoundingBox>();
	if (bounding_box) bounding_box->Invalidate();
}

void ComponentTransform::SetPosition(float3 position_)
{
	position = position_;
	InvalidateHierarchy();
}

void ComponentTransform::SetRotation(Quat rotation_)
{
	rotation = rotation_;
	local_euler_angles = rotation_.ToEulerXYZ().Mul(RADTODEG);
	InvalidateHierarchy();
}

void ComponentTransform::SetRotation(float3 rotation_)
{
	rotation = Quat::FromEulerXYZ(rotation_.x * DEGTORAD, rotation_.y * DEGTORAD, rotation_.z * DEGTORAD);
	local_euler_angles = rotation_;
	InvalidateHierarchy();
}

void ComponentTransform::SetScale(float3 scale_)
{
	scale = scale_;
	InvalidateHierarchy();
}

void ComponentTransform::CalculateGlobalMatrix(bool force)
{
	if (force || dirty)
	{
		local_matrix = float4x4::FromTRS(position, rotation, scale);

		GameObject* parent = GetOwner().GetParent();
		if (parent != nullptr)
		{
			ComponentTransform* parent_transform = parent->GetComponent<ComponentTransform>();

			global_matrix = parent_transform->global_matrix * local_matrix;
		}
		else
		{
			global_matrix = local_matrix;
		}

		dirty = false;
	}
}

float3 ComponentTransform::GetPosition() const
{
	return position;
}

Quat ComponentTransform::GetRotation() const
{
	return rotation;
}

float3 ComponentTransform::GetScale() const
{
	return scale;
}

const float4x4& ComponentTransform::GetLocalMatrix() const
{
	return local_matrix;
}

const float4x4& ComponentTransform::GetGlobalMatrix() const
{
	return global_matrix;
}

ImGuizmo::OPERATION ComponentTransform::GetGizmoOperation() const
{
	return current_guizmo_operation;
}

ImGuizmo::MODE ComponentTransform::GetGizmoMode() const
{
	return current_guizmo_mode;
}

bool ComponentTransform::GetUseSnap() const
{
	return use_snap;
}

float3 ComponentTransform::GetSnap()
{
	return float3(snap[0], snap[1], snap[2]);
}

bool ComponentTransform::GetDirty() const
{
	return dirty;
}