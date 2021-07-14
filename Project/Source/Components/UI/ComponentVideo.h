#pragma once
#include "Components/Component.h"

class ComponentTransform2D;

class ComponentVideo : public Component {
public:
	REGISTER_COMPONENT(ComponentVideo, ComponentType::VIDEO, false); // Refer to ComponentType for the Constructor

	~ComponentVideo(); // Destructor

	void Init() override;							// Inits the component
	void Update() override;							// Update
	void OnEditorUpdate() override;					// Works as input of the AlphaTransparency, color and Texture and Shader used
	void Save(JsonValue jComponent) const override; // Serializes object
	void Load(JsonValue jComponent) override;		// Deserializes object

	void Draw(ComponentTransform2D* transform) const; // Draws the image ortographically using the active camera, and the transform passed as model. It will apply AlphaTransparency if true, and will get Button's additional color to apply if needed
};
