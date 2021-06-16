#pragma once

#include "Scripting/Script.h"

class TrailScript : public Script
{
	GENERATE_BODY(TrailScript);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith) override;

public:
	int speed = 200;
	float life = 0.2f;

private:
	float restLife = 0.0f;
	bool itsVertical = false;
	bool firstTime = false;
};
