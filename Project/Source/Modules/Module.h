#pragma once

enum class UpdateStatus;
struct TesseractEvent;

class Module {
public:
	virtual ~Module();

	// ------- Core Functions ------ //
	// Note that this functions will be executed for each Module in the same order as the Module is appended to the modules list in Application.cpp.
	virtual bool Init();						  // Module initialisations performed before the Main Loop.
	virtual bool Start();						  // Second phase of initialisation. Here will be included all the actions that depend on another module or library to be initalised.
	virtual UpdateStatus PreUpdate();			  // First phase of the update loop. Actions performed before rendering the scene.
	virtual UpdateStatus Update();				  // Second phase of the update loop. Actions performed to render the scene.
	virtual UpdateStatus PostUpdate();			  // Third phase of the update loop. Actions performed after rendering the scene.
	virtual bool CleanUp();						  // Called on quitting the application. Releases recursively all the memory allocated for each Module.
	virtual void ReceiveEvent(TesseractEvent& e); // Called whenever ModuleEventSystem pops an event from the queue (that this module is a listener to)
};
