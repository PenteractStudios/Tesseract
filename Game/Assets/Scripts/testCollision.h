#pragma once

#include "Scripting/Script.h"

class testCollision : public Script
{
	GENERATE_BODY(testCollision);

public:

	void Start() override;
	void Update() override;
	void OnCollision();
};

