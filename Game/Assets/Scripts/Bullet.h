#pragma once

#include "Scripting/Script.h"

class Bullet : public Script
{
	GENERATE_BODY(Bullet);

public:

	void Start() override;
	void Update() override;
	void OnCollision(GameObject& collidedWith) override;

};

