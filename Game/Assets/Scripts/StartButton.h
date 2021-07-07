#pragma once

#include "Scripting/Script.h"
#include "UIAudioType.h"

class GameObject;
class ComponentSelectable;
class ComponentAudioSource;
class SceneTransition;

class StartButton : public Script
{
	GENERATE_BODY(StartButton);

public:

	void Start() override;
	void Update() override;
	void OnButtonClick() override;
	void PlayAudio(UIAudio type);

public:
	UID sceneUID = 0;
	UID transitionUID = 0;

	int checkpointNum = -1;

private:
	GameObject* player = nullptr;

	GameObject* transitionGO = nullptr;
	SceneTransition* sceneTransition = nullptr;

	/* Audio */
	bool playHoveredAudio = true;
	ComponentSelectable* selectable = nullptr;
	ComponentAudioSource* audios[static_cast<int>(UIAudio::TOTAL)] = { nullptr };
};
