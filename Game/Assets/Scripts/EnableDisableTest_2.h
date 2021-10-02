#pragma once

#include "Scripting/Script.h"

class EnableDisableTest_2 : public Script
{
	GENERATE_BODY(EnableDisableTest_2);

public:

	void Start() override;
	void Update() override;

	void OnEnable() override;

	void OnDisable() override;

};

