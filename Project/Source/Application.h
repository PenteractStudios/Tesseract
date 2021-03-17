#pragma once

#include "Utils/MSTimer.h"

#include <vector>

enum class UpdateStatus;

class Module;
class ModuleHardwareInfo;
class ModuleSceneRender;
class ModuleRender;
class ModuleEditor;
class ModuleCamera;
class ModuleWindow;
class ModuleResources;
class ModuleFiles;
class ModuleInput;
class ModulePrograms;
class ModuleDebugDraw;
class ModuleScene;
class ModuleTime;
class ModuleAudio;

class Application {
public:
	Application();
	~Application();

	bool Init();					// Calls the functionality initialisation for all the Modules.
	bool Start();					// Calls a second step functionality initialisation for all the Modules, that needs to be Started after the modules have been Initialised.
	UpdateStatus Update();			// Calls the Update functions for all the modules. The UpdateStatus returned will be used by the main loop to know if the app is running correctly and when to quit.
	bool CleanUp();					// Calls the memory cleaning methods of each module, to release all the memory used by the app before quitting

	void RequestBrowser(char* url);	// Opens the url passed as parameter in a new window your default Internet Borwser

public:
	// ---- Application Modules ---- //
	ModuleHardwareInfo* hardware = nullptr;
	ModuleResources* resources = nullptr;
	ModuleRender* renderer = nullptr;
	ModuleCamera* camera = nullptr;
	ModuleWindow* window = nullptr;
	ModuleFiles* files = nullptr;
	ModuleInput* input = nullptr;
	ModuleEditor* editor = nullptr;
	ModulePrograms* programs = nullptr;
	ModuleDebugDraw* debugDraw = nullptr;
	ModuleScene* scene = nullptr;
	ModuleTime* time = nullptr;
	ModuleAudio* audio = nullptr;

	// - Application Configuration - //
	char appName[20] = "Tesseract";
	char organization[20] = "";

private:
	std::vector<Module*> modules;
};

extern Application* App;
