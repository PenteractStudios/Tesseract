#pragma once

#include "Components/Component.h"
#include "UI/Interfaces/IMouseClickHandler.h"

#include "Math/float4.h"

class ComponentImage;

class ComponentToggle : public Component
	, public IMouseClickHandler {
public:
	REGISTER_COMPONENT(ComponentToggle, ComponentType::TOGGLE, false);

	void OnClicked() override;		   // Action to perform when the button is clicked
	void OnClickedInternal() override; // Action to perform when the button is clicked
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	void OnEditorUpdate() override;
	void Update() override;
	void OnValueChanged(); //Method that triggers OnToggled for scripts and modifies the state (active) of enabledImageObject (if found)

	//Setters
	TESSERACT_ENGINE_API void SetChecked(bool b);
	void SetEnabledImageObj(UID enabledImageObjID_); //Setter for image that will be drawn when isChecked is true
	void SetClicked(bool clicked_);

	//Getters
	bool IsClicked() const;
	float4 GetTintColor() const;  // Returns the correspondant color of the current state
	float4 GetClickColor() const; // Returns colorClicked
	TESSERACT_ENGINE_API bool IsChecked() const;
	ComponentImage* GetEnabledImage() const; //Getter for image that will be drawn when isChecked is true

private:
	bool clicked = false;									//Is currently being clicked (Not saved nor stored)
	bool isChecked = true;									//bool value for wether the checkbox is marked or not
	UID enabledImageObjectID = 0;							//UID to referenced image that will be drawn when isChecked is true
	float4 colorClicked = float4(0.64f, 0.64f, 0.64f, 1.f); // The color when the button is clicked
};
