#pragma once

#include "Scripting/Script.h"
#include "UIAudioType.h"

class ComponentAudioSource;
class ComponentSelectable;

class Resume : public Script
{
	GENERATE_BODY(Resume);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick() override;
	void PlayAudio(UIAudio type);

public:
	UID pauseUID;
	UID hudUID;

private:
	GameObject* pauseCanvas = nullptr;
	GameObject* hudCanvas = nullptr;

	bool playHoveredAudio = true;
	ComponentSelectable* selectable = nullptr;
	ComponentAudioSource* audios[static_cast<int>(UIAudio::TOTAL)] = { nullptr };
};

