#pragma once

#include "Application.h"
#include "Component.h"
#include "AnimationController.h"
#include "AnimationInterpolation.h"
#include "State.h"
#include "Modules/ModuleResources.h"
#include "Resources/ResourceStateMachine.h"
#include "Resources/ResourceClip.h"
#include "Utils/UID.h"
#include <string>
#include "StateMachineEnum.h"

#include <unordered_map>

class GameObject;
class ResourceAnimation;
class ResourceTransition;

class ComponentAnimation : public Component {
public:
	REGISTER_COMPONENT(ComponentAnimation, ComponentType::ANIMATION, false); // Refer to ComponentType for the Constructor

	void Update() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void OnUpdate();

	TESSERACT_ENGINE_API void SendTrigger(const std::string& trigger); // Method to trigger the change of state
	TESSERACT_ENGINE_API void SendTriggerSecondary(const std::string& trigger); // Method to trigger the change of state

	TESSERACT_ENGINE_API State* GetCurrentState() {
		if (!loadedResourceStateMachine || currentStatePrincipal.id == 0) return nullptr;
		return &currentStatePrincipal;
	}
	TESSERACT_ENGINE_API void SetCurrentState(State* mCurrentState) {
		currentStatePrincipal = *mCurrentState;
	}

	TESSERACT_ENGINE_API State* GetCurrentStateSecondary() {
		if (!loadedResourceStateMachineSecondary || currentStateSecondary.id == 0) return nullptr;
		return &currentStateSecondary;
	}
	TESSERACT_ENGINE_API void SetCurrentStateSecondary(State* mCurrentState) {
		currentStateSecondary = *mCurrentState;
	}

public:
	UID stateMachineResourceUIDPrincipal = 0;
	UID stateMachineResourceUIDSecondary = 0;
	State currentStatePrincipal;
	State currentStateSecondary;

private:
	void UpdateAnimations(GameObject* gameObject);
	void LoadResourceStateMachine(UID stateMachineResourceUid, StateMachineEnum stateMachineEnum);
	void InitCurrentTimeStates(UID stateMachineResourceUid, StateMachineEnum stateMachineEnum);
	bool loadedResourceStateMachine = false;
	bool loadedResourceStateMachineSecondary = false;

private:
	std::list<AnimationInterpolation> animationInterpolationsPrincipal; //List of the current interpolations between states
	std::list<AnimationInterpolation> animationInterpolationsSecondary; //List of the current interpolations between states
	std::unordered_map<UID, float> currentTimeStatesPrincipal;
	std::unordered_map<UID, float> currentTimeStatesSecondary;

	std::unordered_map<UID, std::unordered_map<unsigned int, EventClip>> listClipsKeyEvents;
};
