#pragma once

#include "Scripting/Script.h"

class ExposeTest : public Script
{
	GENERATE_BODY(ExposeTest);

public:

	void Start() override;
	void Update() override;

public:

	bool test = false;
	long long padding = -1L;
	UID prefabId = 0;
};

