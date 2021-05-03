#include "ModuleHardwareInfo.h"

#include "Globals.h"
#include "Application.h"

#include "SDL_version.h"
#include "SDL_cpuinfo.h"
#include "SDL_video.h"
#include "GL/glew.h"
#include "IL/il.h"
#include "assimp/version.h"
#include "Brofiler.h"
#include "LinearMath/btScalar.h"

#include "Utils/Leaks.h"

bool ModuleHardwareInfo::Start() {
	SDL_version sdlVersionStruct;
	SDL_VERSION(&sdlVersionStruct);

	sprintf_s(glewVersion, "%i.%i.%i", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
	sprintf_s(sdlVersion, "%i.%i.%i", sdlVersionStruct.major, sdlVersionStruct.minor, sdlVersionStruct.patch);
	sprintf_s(assimpVersion, "%i.%i.%i", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	sprintf_s(devilVersion, "%i.%i.%i", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
	sprintf_s(bulletVersion, "%i.%i", btGetVersion() / 100, (btGetVersion() % 100));

	cpuCount = SDL_GetCPUCount();
	cacheSizeKb = SDL_GetCPUCacheLineSize();
	ramGb = SDL_GetSystemRAM() / 1000.0f;
	caps[0] = SDL_Has3DNow();
	caps[1] = SDL_HasARMSIMD();
	caps[2] = SDL_HasAVX();
	caps[3] = SDL_HasAVX2();
	caps[4] = SDL_HasAVX512F();
	caps[5] = SDL_HasAltiVec();
	caps[6] = SDL_HasMMX();
	caps[7] = SDL_HasNEON();
	caps[8] = SDL_HasRDTSC();
	caps[9] = SDL_HasSSE();
	caps[10] = SDL_HasSSE2();
	caps[11] = SDL_HasSSE3();
	caps[12] = SDL_HasSSE41();
	caps[13] = SDL_HasSSE42();

	gpuVendor = (const char*) glGetString(GL_VENDOR);
	gpuRenderer = (const char*) glGetString(GL_RENDERER);
	gpuOpenglVersion = (const char*) glGetString(GL_VERSION);

	return true;
}

UpdateStatus ModuleHardwareInfo::Update() {
	BROFILER_CATEGORY("ModuleHardwareInfo - Update", Profiler::Color::Orange)

	int vramBudgetKb;
	int vramAvailableKb;
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &vramBudgetKb);
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &vramAvailableKb);
	vramBudgetMb = vramBudgetKb / 1000.0f;
	vramAvailableMb = vramAvailableKb / 1000.0f;
	vramUsageMb = vramBudgetMb - vramAvailableMb;

	return UpdateStatus::CONTINUE;
}

bool ModuleHardwareInfo::CleanUp() {
	return true;
}
