#pragma once

#include "Components/Component.h"

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

#if defined(TESSERACT_ENGINE_API)
/* do nothing. */
#elif defined(_MSC_VER)
#define TESSERACT_ENGINE_API __declspec(dllexport)
#endif

// TODO: ANCHOR ARE TAKEN INTO ACCOUNT IN ANY WAY, NOR ARE THEY MODIFIABLE LIKE THEY WOULD IN UNITY
// ANCHOR PRESETS CREATED BUT NOT WORKING -> WOLUD BE FIXED IN OTHER BRANCH

// Transform in the 2D Space. Used for UI Components.
class ComponentTransform2D : public Component {
public:
	REGISTER_COMPONENT(ComponentTransform2D, ComponentType::TRANSFORM2D, false);
	void Update() override;							// Calculates the Global Matrix
	void OnEditorUpdate() override;					// Works as input of the different values of the component
	void Save(JsonValue jComponent) const override; // Serializes Component
	void Load(JsonValue jComponent) override;		// Deserializes Component
	void DrawGizmos() override;
	bool CanBeRemoved() const override; //Returns false if any UI Elements are found in this GameObject or its children

	TESSERACT_ENGINE_API void SetPosition(float3 position);											  // Sets this position to value
	TESSERACT_ENGINE_API void SetSize(float2 size);													  // Sets this size to value
	void SetRotation(Quat rotation);																  // Sets this rotation to value and calculates Euler Angles rotation
	void SetRotation(float3 rotation);																  // Sets this eulerAngles to value and calculates Quat rotation
	void SetScale(float3 scale);																	  // Sets this scale to value
	void SetAnchorMin(float2 anchorMin);															  // Sets this anchorMin to value
	void SetAnchorMax(float2 anchorMax);															  // Sets this anchorMax to value
	void SetPivot(float2 pivotPosition);															  // Sets this pivot to value
	void UpdatePivotPosition();																		  // Update this pivot position to value
	const float4x4 GetGlobalMatrix() const;															  // Returns GlobalMatrix
	const float4x4 GetGlobalScaledMatrix(bool scaleFactored = true, bool view3DActive = false) const; // Returns GlobalMatrix with the size of the item. scaleFactored being true means that the canvas size reference will be used to scale (factor). view3DActive is true when the Editor is on 3D Mode and will return the global downscaled to have a proper 3D View.

	TESSERACT_ENGINE_API float3 GetPosition() const; // Returns the position
	TESSERACT_ENGINE_API float2 GetSize() const;	 // Returns the size
	float3 GetScale() const;						 // Returns the scale
	float3 GetPivotPosition() const;			 // Returns the pivot position

	void InvalidateHierarchy();							 // Invalidates hierarchy
	void Invalidate();									 // Invalidates component
	void DuplicateComponent(GameObject& owner) override; // Duplicates component (THIS SHOULDN'T BE USED)

private:
	void CalculateGlobalMatrix();								  // Calculates the Global Matrix
	void UpdateUIElements();									  // If the transform changes, is gonna update UI Elements that need to recalculate vertices (p.e: ComponentText RecalculateVertices)
	bool HasAnyUIElementsInChildren(const GameObject* obj) const; //Returns true if any UI Elements are found in this GameObject or its children

private:
	float2 pivot = float2(0.5, 0.5);	 // The position of the pivot in 2D
	float3 pivotPosition = float3::zero; // The position of the pivot in the world
	float2 size = float2(200, 200);	 // The size of the item

	float3 position = float3::zero;			// The offset position
	Quat rotation = Quat::identity;			// The rotation of the element in Quaternion
	float3 localEulerAngles = float3::zero; // The rotation of the element in Euler
	float3 scale = float3::one;				// The scale of the element

	float2 anchorMin = float2(0.5, 0.5); // The Anchor Min. Represents the lower left handle.
	float2 anchorMax = float2(0.5, 0.5); // The Anchor Max. Represents the upper right handle.

	float4x4 localMatrix = float4x4::identity;	// Local matrix
	float4x4 globalMatrix = float4x4::identity; // Global Matrix

	bool dirty = true;
};