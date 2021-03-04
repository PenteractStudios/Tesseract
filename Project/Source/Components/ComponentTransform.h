#pragma once

#include "Component.h"

#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"
#include "imgui.h"

class ComponentTransform : public Component {
public:
	REGISTER_COMPONENT(ComponentTransform, ComponentType::TRANSFORM); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	void DuplicateComponent(GameObject& owner) override;

	void InvalidateHierarchy();						// Marks all the hierarchy of the owner GameObject as 'dirty'
	void Invalidate();								// Marks this Transform and the BoundingBox of the GameObject as 'dirty'. Used also in the InvalidateHierarchy() recursion.
	void CalculateGlobalMatrix(bool force = false); // Recalculates the Transformation Matrix from 'position', 'rotation' and 'scale'.

	// ---------- Setters ---------- //
	// This setters will broadcast the OnTransformUpdate() signal to the Components.
	void SetPosition(float3 position);
	void SetRotation(Quat rotation);
	void SetRotation(float3 rotation);
	void SetScale(float3 scale);
	void SetTRS(float4x4& newTransform);

	// ---------- Getters ---------- //
	float3 GetPosition() const;
	Quat GetRotation() const;
	float3 GetScale() const;
	const float4x4& GetLocalMatrix() const;
	const float4x4& GetGlobalMatrix() const;
	bool GetDirty() const;

private:
	float3 position = float3::zero;			// Position of the GameObject in world coordinates.
	Quat rotation = Quat::identity;			// Rotation of the GameObject as a Quaternion.
	float3 localEulerAngles = float3::zero; // Rotation of the GameObject as euler angles.
	float3 scale = float3::one;				// Scale of the GameObject.

	float4x4 localMatrix = float4x4::identity;	// Transform Matrix in local coordinates from its parent gameobject.
	float4x4 globalMatrix = float4x4::identity; // Transform Matrix in world coordinates.

	bool dirty = true; // If set to true CalculateGlobalMatrix() will update the Transform on the next frame. Otherwise, it will skip the calculations.
};
