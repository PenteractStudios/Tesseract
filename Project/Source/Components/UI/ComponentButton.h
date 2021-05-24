#pragma once

#include "ComponentSelectable.h"
#include "UI/Interfaces/IMouseClickHandler.h"

#include "Math/float4.h"

class GameObject;

// Component UI Button that allows user interaction by clicking/pressing the button. Inherits ComponentSelectable and implements IMouseClickHandler.
class ComponentButton : public Component
	, public IMouseClickHandler {
public:
	REGISTER_COMPONENT(ComponentButton, ComponentType::BUTTON, false);

	void Init() override;							// Inits the component
	void OnEditorUpdate() override;					// Works as input of the Colors and the Selectable interface
	void Save(JsonValue jComponent) const override; // Serializes object
	void Load(JsonValue jComponent) override;		// Deserializes object

	void OnClickedInternal() override;				   // Internal Action to perform when the button is clicked
	void OnClicked() override;						   // Action to perform when the button is clicked
	void Update() override;							   // Updates clicked variable if the button is Left-clicked mouse

	bool IsClicked() const;			// Returns true if the button is clicked
	void SetClicked(bool clicked_); // Sets clicked to the value

	float4 GetTintColor() const;	 // Returns the correspondant color of the current state
	float4 GetClickColor() const; // Returns colorClicked

private:
	bool clicked = false; // Clicked state

	float4 colorClicked = float4(0.64f, 0.64f, 0.64f, 1.f); // The color when the button is clicked
};
