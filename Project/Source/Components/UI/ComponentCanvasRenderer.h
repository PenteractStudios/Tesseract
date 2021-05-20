#pragma once

#include "Components/Component.h"

#include "Math/float2.h"

class ComponentCanvas;

class ComponentCanvasRenderer : public Component {
public:
	REGISTER_COMPONENT(ComponentCanvasRenderer, ComponentType::CANVASRENDERER, false);

	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	bool CanBeRemoved() const override;				 //This override returns false if the GameObject holds a ComponentImage/ComponentText
	void Render(const GameObject* gameObject) const; //Calls Draw on all images if one of its parents is a canvas
	void DrawGizmos() override;						 // This override draw the 2D screen

	float2 GetCanvasSize();
	float2 GetScreenReferenceSize() const;
	float GetCanvasScreenFactor();
	ComponentCanvas* AnyParentHasCanvas(GameObject* current) const; //Recursively iterates through parents to check if one of them contains a Canvas Component
};
