#pragma once

#include "Module.h"

struct SDL_version;

class ModuleHardwareInfo : public Module {
public:
	bool Start() override;
	UpdateStatus Update() override;
	bool CleanUp() override;

public:
	char sdlVersion[20] = "Not available";
	char glewVersion[20] = "Not available";
	char assimpVersion[20] = "Not available";
	char devilVersion[20] = "Not available";
	char mathgeolibVersion[20] = "Not available"; //This will be hardcoded. Couldnt find the MACRO.
	char imguiVersion[20] = "Not available";
	char imguizmoVersion[20] = "Not available"; // This one is actually not available
	char fmtVersion[20] = "Not available";
	char freetypeVersion[20] = "Not available";
	char libsndVersion[20] = "Not available";
	char physFSVersion[20] = "Not available";
	char rapidJsonVersion[20] = "Not available";
	char openALVersion[20] = "Not available";

	int cpuCount = 0;
	int cacheSizeKb = 0;
	float ramGb = 0;
	bool caps[14] = {false};
	const char* gpuVendor = "Not available";
	const char* gpuRenderer = "Not available";
	const char* gpuOpenglVersion = "Not available";
	float vramBudgetMb = 0;
	float vramAvailableMb = 0;
	float vramUsageMb = 0;
	float vramReservedMb = 0;
};
