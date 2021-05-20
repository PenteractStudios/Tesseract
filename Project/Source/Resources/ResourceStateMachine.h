#pragma once
#include "State.h"
#include "Transition.h"
#include "Resources/ResourceType.h"
#include "Resources/Resource.h"
#include "Utils/UID.h"

#include <list>
#include <unordered_map>

class ResourceAnimation;
class ResourceStateMachine : public Resource {
public:
	REGISTER_RESOURCE(ResourceStateMachine, ResourceType::STATE_MACHINE);

	void Load() override;
	void Unload() override;
	void SaveToFile(const char* filePath);

	State AddState(const std::string& name, UID clipUid); //Add state to list of states and add clip to list of clips if dosen't contains him
	void AddClip(UID clipUid);
	void AddTransition(const State& from, const State& to, const float interpolation, const std::string& name);
	Transition* FindTransitionGivenName(const std::string& name);

	State* GetInitialState() const {
		return initialState;
	}

	void SetInitialState(State *newState){
		initialState = newState;
	}

public:
	std::list<State> states;
	State* initialState;

private:
	std::list<UID> clipsUids;
	std::unordered_map<std::string, Transition> transitions;
};
