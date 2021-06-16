#pragma once

#include "Scripting/Script.h"

class ReloadScene : public Script {
	GENERATE_BODY(ReloadScene);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick()override;

public:
	
	UID sceneUID = 0;
};

