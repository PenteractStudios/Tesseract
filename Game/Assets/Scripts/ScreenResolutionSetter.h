#pragma once

#include "Scripting/Script.h"

class ComponentText;

 /// <summary>
 /// This class is meant to modify externs delcared inside ScreenResolution confirmer whenever Button is pressed, as well as setting the text (if found)
 /// </summary>
 class ScreenResolutionSetter : public Script {
	GENERATE_BODY(ScreenResolutionSetter);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick() override;

public:
	bool increasing = false;
	UID textObjectID = 0;

private:
	void IncreaseResolution(int multiplier);
	void UpdateText();
private:

	ComponentText* text = nullptr;
};

