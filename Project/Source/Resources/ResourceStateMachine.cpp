#include "ResourceStateMachine.h"
#include "Application.h"
#include "Transition.h"
#include "State.h"

#include "Resources/ResourceClip.h"
#include "Modules/ModuleFiles.h"
#include "FileSystem/JsonValue.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleResources.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/Leaks.h"

#define JSON_TAG_CLIPS "Clips"
#define JSON_TAG_STATES "States"
#define JSON_TAG_TRANSITIONS "Transitions"

#define JSON_TAG_CLIP_ID "ClipId"

#define JSON_TAG_NAME "Name"
#define JSON_TAG_ID "Id"

#define JSON_TAG_SOURCE "Source"
#define JSON_TAG_TARGET "Target"
#define JSON_TAG_TRIGGER_NAME "Trigger"
#define JSON_TAG_INTERPOLATION_DURATION "Interpolation"

void ResourceStateMachine::Load() {
	// Timer to measure loading a material
	MSTimer timer;
	timer.Start();
	std::string filePath = GetResourceFilePath();
	LOG("Loading material from path: \"%s\".", filePath.c_str());

	Buffer<char> buffer = App->files->Load(filePath.c_str());
	if (buffer.Size() == 0) return;

	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return;
	}
	JsonValue jStateMachine(document, document);
	assert(document.IsObject());

	JsonValue clipArray = jStateMachine[JSON_TAG_CLIPS];
	for (int i = 0; i < clipArray.Size(); ++i) {
		UID clipUID = clipArray[i];
		ResourceClip* clip = App->resources->GetResource<ResourceClip>(clipUID);
		App->resources->IncreaseReferenceCount(clipUID);
		clipsUids.push_back(clipUID);
	}

	std::unordered_map<UID, State> stateMap;
	JsonValue stateArray = jStateMachine[JSON_TAG_STATES];
	for (int i = 0; i < stateArray.Size(); ++i) {
		UID id = stateArray[i][JSON_TAG_ID];
		std::string name = stateArray[i][JSON_TAG_NAME];
		UID clipId = stateArray[i][JSON_TAG_CLIP_ID];
		State state(name, clipId, 0, id);
		states.push_back(state);
		stateMap.insert(std::make_pair(id, state));
	}

	JsonValue transitionArray = jStateMachine[JSON_TAG_TRANSITIONS];
	for (int i = 0; i < transitionArray.Size(); ++i) {
		std::string triggerName = transitionArray[i][JSON_TAG_TRIGGER_NAME];
		UID id = transitionArray[i][JSON_TAG_ID];
		UID source = transitionArray[i][JSON_TAG_SOURCE];
		UID target = transitionArray[i][JSON_TAG_TARGET];
		float interpolationDuration = transitionArray[i][JSON_TAG_INTERPOLATION_DURATION];
		Transition transition(stateMap.find(source)->second, stateMap.find(target)->second, interpolationDuration, id);
		transitions.insert(std::make_pair(triggerName, transition));
	}

	unsigned timeMs = timer.Stop();
	LOG("Material loaded in %ums", timeMs);
}

void ResourceStateMachine::Unload() {
	std::list<UID>::iterator itClip;
	for (itClip = clipsUids.begin(); itClip != clipsUids.end(); ++itClip) {
		App->resources->DecreaseReferenceCount((*itClip));
	}
}

void ResourceStateMachine::SaveToFile(const char* filePath) {
	LOG("Saving ResourceStateMachine to path: \"%s\".", filePath);

	MSTimer timer;
	timer.Start();

	// Create document
	rapidjson::Document document;
	JsonValue jStateMachine(document, document);

	// Save JSON values
	document.SetObject();

	// Saving Clips UIDs
	JsonValue clipArray = jStateMachine[JSON_TAG_CLIPS];
	int i = 0;
	std::list<UID>::iterator itClip;
	for (itClip = clipsUids.begin(); itClip != clipsUids.end(); ++itClip) {
		clipArray[i] = (*itClip);
		++i;
	}

	// Saving States
	JsonValue stateArray = jStateMachine[JSON_TAG_STATES];
	i = 0;
	std::list<State>::iterator itState;
	for (itState = states.begin(); itState != states.end(); ++itState) {
		stateArray[i][JSON_TAG_ID] = (*itState).id;
		stateArray[i][JSON_TAG_NAME] = (*itState).name.c_str();
		stateArray[i][JSON_TAG_CLIP_ID] = (*itState).clipUid;
		++i;
	}

	//Saving transitions
	JsonValue transitionArray = jStateMachine[JSON_TAG_TRANSITIONS];
	i = 0;
	for (const auto& element : transitions) {
		transitionArray[i][JSON_TAG_TRIGGER_NAME] = element.first.c_str();
		transitionArray[i][JSON_TAG_ID] = element.second.id;
		transitionArray[i][JSON_TAG_SOURCE] = element.second.source.id;
		transitionArray[i][JSON_TAG_TARGET] = element.second.target.id;
		transitionArray[i][JSON_TAG_INTERPOLATION_DURATION] = element.second.interpolationDuration;
		++i;
	}

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	bool saved = App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save material resource.");
		return;
	}

	unsigned timeMs = timer.Stop();
	LOG("Material saved in %ums", timeMs);
}

State ResourceStateMachine::AddState(const std::string& name, UID clipUID) {
	State state(name, clipUID);
	states.push_back(state);
	AddClip(clipUID);

	return state;
}

void ResourceStateMachine::AddClip(UID clipUid) {
	if (std::find(clipsUids.begin(), clipsUids.end(), clipUid) == clipsUids.end()) {
		clipsUids.push_back(clipUid);
	}
}

void ResourceStateMachine::AddTransition(const State& from, const State& to, const float interpolation, const std::string& name) {
	//Checking for unique name
	Transition* transition = FindTransitionGivenName(name);
	if (transition) {
		return;
	}

	transitions.insert(std::make_pair(name, Transition(from, to, interpolation)));
}

Transition* ResourceStateMachine::FindTransitionGivenName(const std::string& name) {
	if (transitions.empty()) {
		return nullptr;
	}

	std::unordered_map<std::string, Transition>::iterator it = transitions.find(name);
	if (it != transitions.end()) {
		return &(*it).second;
	}

	return nullptr;
}
