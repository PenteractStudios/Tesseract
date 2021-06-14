#pragma once

#include "Scripting/Script.h"

class gg : public Script
{
	GENERATE_BODY(gg);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith) override;

	UID scene;

};

