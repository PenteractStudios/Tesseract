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

//TODO THIS CLASS CURRENTLY ONLY WORKS FOR OBJECTS THAT USE THE CENTER OF THE SCREEN AS THE CENTER OF COORDINATES,
//NEITHER PIVOT NOR ANCHOR ARE TAKEN INTO ACCOUNT IN ANY WAY, NOR ARE THEY MODIFIABLE LIKE THEY WOULD IN UNITY

struct AnchorPreset {
	enum class AnchorPresetType {
		// Presets
		TOP_HORIZONTAL_LEFT_VERTICAL,
		TOP_HORIZONTAL_CENTER_VERTICAL,
		TOP_HORIZONTAL_RIGHT_VERTICAL,
		TOP_HORIZONTAL_STRETCH_VERTICAL,

		MIDDLE_HORIZONTAL_LEFT_VERTICAL,
		MIDDLE_HORIZONTAL_CENTER_VERTICAL,
		MIDDLE_HORIZONTAL_RIGHT_VERTICAL,
		MIDDLE_HORIZONTAL_STRETCH_VERTICAL,

		BOTTOM_HORIZONTAL_LEFT_VERTICAL,
		BOTTOM_HORIZONTAL_CENTER_VERTICAL,
		BOTTOM_HORIZONTAL_RIGHT_VERTICAL,
		BOTTOM_HORIZONTAL_STRETCH_VERTICAL,

		STRETCH_HORIZONTAL_LEFT_VERTICAL,
		STRETCH_HORIZONTAL_CENTER_VERTICAL,
		STRETCH_HORIZONTAL_RIGHT_VERTICAL,
		STRETCH_HORIZONTAL_STRETCH_VERTICAL,

		// Custom
		CUSTOM
	};

	AnchorPreset(AnchorPresetType type_, float2 anchorMin_, float2 anchorMax_)
		: type(type_)
		, anchorMin(anchorMin_)
		, anchorMax(anchorMax_) {
	}

	AnchorPresetType type;
	float2 anchorMin;
	float2 anchorMax;
};

struct Rect {
	Rect(float top_, float bottom_, float left_, float right_)
		: top(top_)
		, bottom(bottom_)
		, left(left_)
		, right(right_) {}

	float top, bottom, left, right;
};

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

	TESSERACT_ENGINE_API void SetPosition(float3 position); // Sets this position to value
	TESSERACT_ENGINE_API void SetSize(float2 size);			// Sets this size to value
	void SetRotation(Quat rotation);						// Sets this rotation to value and calculates Euler Angles rotation
	void SetRotation(float3 rotation);						// Sets this eulerAngles to value and calculates Quat rotation
	void SetScale(float3 scale);							// Sets this scale to value
	void SetAnchorMin(float2 anchorMin);					// Sets this anchorMin to value
	void SetAnchorMax(float2 anchorMax);					// Sets this anchorMax to value
	void SetPivot(float2 pivotPosition);					// Sets this pivot to value
	void UpdatePivotPosition();								// Update this pivot position to value
	const float4x4 GetGlobalMatrix();						// Returns GlobalMatrix
	const float4x4 GetGlobalScaledMatrix();					// Returns GlobalMatrix with the size of the item
	const float4x4 GetGlobalRotatedInPivotMatrix();			// Returns GlobalMatrix with the rotation of the item in a pivot position
	void UpdateTransformChanges();							// Update the tranform matrix
	Quat GetGlobalRotation() const;							// Returns the accumulated rotation of the parents and the current component

	TESSERACT_ENGINE_API float3 GetPosition() const; // Returns the position
	TESSERACT_ENGINE_API float2 GetSize() const;	 // Returns the size
	float3 GetScale() const;						 // Returns the scale
	float3 GetPivotPosition() const;				 // Returns the pivot position
	float3 GetPositionRelativeToParent() const;
	float3 GetScreenPosition() const;

	void InvalidateHierarchy(); // Invalidates hierarchy
	void Invalidate();			// Invalidates component

	void SetTop(float top);		  // Sets the right anchor
	void SetBottom(float bottom); // Sets the right anchor
	void SetLeft(float left);	  // Sets the right anchor
	void SetRight(float right);	  // Sets the right anchor

private:
	void CalculateGlobalMatrix();								  // Calculates the Global Matrix
	void UpdateUIElements();									  // If the transform changes, is gonna update UI Elements that need to recalculate vertices (p.e: ComponentText RecalculateVertices)
	bool HasAnyUIElementsInChildren(const GameObject* obj) const; //Returns true if any UI Elements are found in this GameObject or its children

private:
	float3 position = float3::zero;			// The offset position
	Quat rotation = Quat::identity;			// The rotation of the element in Quaternion
	float3 localEulerAngles = float3::zero; // The rotation of the element in Euler
	float3 scale = float3::one;				// The scale of the element

	float2 pivot = float2(0.5, 0.5);	 // The position of the pivot in 2D
	float3 pivotPosition = float3::zero; // The position of the pivot in the world
	float2 size = float2(200, 200);		 // The size of the item

	Rect anchorsRect = Rect(0, 0, 0, 0);																			   // Positions of the rectangles edges relative to their anchors.
	float2 anchorMin = float2(0.5, 0.5);																			   // The Anchor Min. Represents the lower left handle.
	float2 anchorMax = float2(0.5, 0.5);																			   // The Anchor Max. Represents the upper right handle.
	static std::array<AnchorPreset, 16> anchorPresets;																   // Listwith all the possible anchors presets
	AnchorPreset::AnchorPresetType anchorSelected = AnchorPreset::AnchorPresetType::MIDDLE_HORIZONTAL_CENTER_VERTICAL; // The anchor selected
	bool isCustomAnchor = false;																					   // Variable that checks if is selected Custom or Presets in the inspector

	float4x4 localMatrix = float4x4::identity;	// Local matrix
	float4x4 globalMatrix = float4x4::identity; // Global Matrix

	bool dirty = true;
};