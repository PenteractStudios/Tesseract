#pragma once

#include "Components/Component.h"

#include "Math/float4.h"

class ComponentTransform2D;

// Component that renders an Image on a Quad
class ComponentImage : public Component {
public:
	REGISTER_COMPONENT(ComponentImage, ComponentType::IMAGE, false);

	~ComponentImage(); // Destructor

	void Init() override;							// Inits the component
	void Update() override;							// Update
	void OnEditorUpdate() override;					// Works as input of the AlphaTransparency, color and Texture and Shader used
	void Save(JsonValue jComponent) const override; // Serializes object
	void Load(JsonValue jComponent) override;		// Deserializes object
	void DuplicateComponent(GameObject& owner) override;

	void Draw(const ComponentTransform2D* transform) const; // Draws the image ortographically using the active camera, and the transform passed as model. It will apply AlphaTransparency if true, and will get Button's additional color to apply if needed
	void SetColor(float4 color_);

private:
	float4 GetTintColor() const; // Gets an additional color that needs to be applied to the image. Currently gets the color of the Button

private:
	float4 color = float4::one;		// Color used as default tainter
	bool alphaTransparency = false; // Enables Alpha Transparency of the image and the color
	UID textureID = 0;				// ID of the image
};
