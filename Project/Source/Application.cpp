#include "Application.h"

#include "Globals.h"
#include "Utils/Logging.h"
#include "Modules/ModuleHardwareInfo.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleWindow.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleAudio.h"
#include "Modules/ModuleProject.h"
#include "Modules/ModuleEvents.h"
#include "Modules/ModuleUserInterface.h"

#include "SDL_timer.h"
#include <windows.h>
#include "Brofiler.h"

#include "Utils/Leaks.h"

Application::Application() {
	// Order matters: they will Init/start/update in this order
	modules.push_back(events = new ModuleEvents());

	modules.push_back(hardware = new ModuleHardwareInfo());
	modules.push_back(window = new ModuleWindow());
	modules.push_back(files = new ModuleFiles());
	modules.push_back(project = new ModuleProject());
	modules.push_back(resources = new ModuleResources());
	modules.push_back(programs = new ModulePrograms());
	modules.push_back(audio = new ModuleAudio());

	modules.push_back(time = new ModuleTime());
	modules.push_back(input = new ModuleInput());
	modules.push_back(camera = new ModuleCamera());

	modules.push_back(scene = new ModuleScene());
	modules.push_back(userInterface = new ModuleUserInterface());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(debugDraw = new ModuleDebugDraw());

	modules.push_back(renderer = new ModuleRender());
}

Application::~Application() {
	for (Module* module : modules) {
		delete module;
	}
}

bool Application::Init() {
	bool ret = true;

	for (std::vector<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it) {
		ret = (*it)->Init();
	}

	return ret;
}

bool Application::Start() {
	bool ret = true;

	for (std::vector<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it) {
		ret = (*it)->Start();
	}

	return ret;
}

UpdateStatus Application::Update() {
	BROFILER_CATEGORY("App - Update", Profiler::Color::Red)

	UpdateStatus ret = UpdateStatus::CONTINUE;

	for (std::vector<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UpdateStatus::CONTINUE; ++it) {
		ret = (*it)->PreUpdate();
	}

	for (std::vector<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UpdateStatus::CONTINUE; ++it) {
		ret = (*it)->Update();
	}

	for (std::vector<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UpdateStatus::CONTINUE; ++it) {
		ret = (*it)->PostUpdate();
	}

	time->WaitForEndOfFrame();

	return ret;
}

bool Application::CleanUp() {
	bool ret = true;

	for (std::vector<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it) {
		ret = (*it)->CleanUp();
	}

	return ret;
}

void Application::RequestBrowser(char* url) {
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}
