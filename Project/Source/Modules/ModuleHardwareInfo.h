#pragma once

#include "Module.h"

struct SDL_version;

class ModuleHardwareInfo : public Module {
public:
	bool Start() override;
	UpdateStatus Update() override;
	bool CleanUp() override;

public:
	char glewVersion[20] = "Not available";
	char sdlVersion[20] = "Not available";
	char assimpVersion[20] = "Not available";
	char devilVersion[20] = "Not available";
	char bulletVersion[20] = "Not Available";
	// TODO: fmt, imgui, imguizmo, libsndfile, mathgeolib, openAL
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
