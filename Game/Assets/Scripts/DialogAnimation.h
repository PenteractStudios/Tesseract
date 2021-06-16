#pragma once

#include "Scripting/Script.h"

class DialogAnimation : public Script
{
	GENERATE_BODY(DialogAnimation);

public:
	void Start() override;
	void Update() override;

public:
	float3 startPosition = float3(0, 0, 0);
	float3 endPosition = float3(0, 0, 0);
	float appearAnimationTime = .5f;
	float disappearAnimationTime = .5f;
	float currentTime = 0.0f;
	float duration = 5.0f;

private:
	void TransitionUIElementsColor(bool appearing = true, bool mustLerp = true);
	void RetrieveUIComponents(GameObject* current);

private:
	bool startAnimationFinished = false;
	bool endAnimationFinished = false;
	std::vector<Component*> uiComponents;
	std::vector<float4> uiColors;
};

