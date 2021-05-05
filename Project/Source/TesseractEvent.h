#pragma once

#include <string>
#include <variant>

#include "Math/float2.h"

class GameObject;
class Component;
class Resource;

struct AssetFolder;

#define EventVariant std::variant<int, DestroyGameObjectStruct, AddResourceStruct, UpdateFoldersStruct, ChangeSceneStruct>

/* Creating a new event type:
*    1. Add a new EventType for the new event (ALWAYS ABOVE COUNT)
*    2. Add a struct containing the necessary information for said event inside the union below (Make sure it has a constructor)
*    3. (If allocating) Make sure you release all allocated resources in ModuleEvents.cpp's CleanUpEvent()
*	 4. Remember to make an std::emplace whenever generating a TesseractEvent, there are no default values so trying to std::get a non initiated variant will return a crash
*/

enum class TesseractEventType {
	UNKNOWN = 0,
	GAMEOBJECT_DESTROYED,
	PRESSED_PLAY,
	PRESSED_PAUSE,
	PRESSED_RESUME,
	PRESSED_STEP,
	PRESSED_STOP,
	ADD_RESOURCE,
	UPDATE_FOLDERS,
	MOUSE_CLICKED,
	MOUSE_RELEASED,
	CHANGE_SCENE,
	RESOURCES_LOADED,
	COMPILATION_FINISHED,
	SCREEN_RESIZED,
	COUNT
};

struct AddResourceStruct {
	Resource* resource = nullptr;
	AddResourceStruct(Resource* resource_)
		: resource(resource_) {}
};

struct DestroyGameObjectStruct {
	GameObject* gameObject = nullptr;
	DestroyGameObjectStruct(GameObject* gameObject_)
		: gameObject(gameObject_) {}
};

struct UpdateFoldersStruct {
	AssetFolder* folder = nullptr;
	UpdateFoldersStruct(AssetFolder* folder_)
		: folder(folder_) {
	}
};

struct ChangeSceneStruct {
	const char* scenePath = nullptr;
	ChangeSceneStruct(const char* scenePath_)
		: scenePath(scenePath_) {}
};

struct TesseractEvent {
public:
	TesseractEvent(TesseractEventType type);

public:
	TesseractEventType type = TesseractEventType::UNKNOWN;

	template<typename T>
	T& Get();

	template<typename T, typename... Args>
	void Set(Args... args);

private:
	EventVariant variant;
};

template<typename T>
inline T& TesseractEvent::Get() {
	return std::get<T>(variant);
}

template<typename T, typename... Args>
inline void TesseractEvent::Set(Args... args) {
	variant.emplace<T>(std::forward<Args>(args)...);
}