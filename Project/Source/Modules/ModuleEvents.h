#pragma once

#include "TesseractEvent.h"
#include "Modules/Module.h"

#include <concurrent_queue.h>
#include <vector>
#include <unordered_map>

class ModuleEvents : public Module {
public:
	bool Init() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;
	bool CleanUp() override;

	//All events should be added on the Start method for security pruposes,
	//exceptions are those Modules that MUST handle some events on last instance,
	//such as ModuleScene, which must handle GameObjectDestroyed the last so that
	//there are no nullptrs
	void AddObserverToEvent(TesseractEventType type, Module* moduleToAdd);
	void RemoveObserverFromEvent(TesseractEventType type, Module* moduletoRemove);

	void AddEvent(const TesseractEvent& newEvent);

private:
	void ProcessEvents();
	void ProcessEvent(TesseractEvent& e);

private:
	concurrency::concurrent_queue<TesseractEvent> eventQueue;
	std::vector<Module*> observerArray[(int) TesseractEventType::COUNT];
};
