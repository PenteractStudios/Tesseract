#include "ModuleHardwareInfo.h"

#include "Globals.h"
#include "Application.h"

#include "SDL_version.h"
#include "SDL_cpuinfo.h"
#include "SDL_video.h"
#include "GL/glew.h"
#include "assimp/version.h"
#include "IL/il.h"
#include "imgui.h"
//#include mathgeolib
#include "fmt/core.h"
#include "freetype/freetype.h"
#include "sndfile.h"
#include "physfs.h"
#include "rapidjson/rapidjson.h"
#include "AL/al.h"
extern "C" {
#include "libavutil/version.h"
}

#include "Brofiler.h"
#include "LinearMath/btScalar.h"

#include "Utils/Leaks.h"

bool ModuleHardwareInfo::Start() {
	SDL_version sdlVersionStruct;
	SDL_VERSION(&sdlVersionStruct);

	sprintf_s(sdlVersion, "%i.%i.%i", sdlVersionStruct.major, sdlVersionStruct.minor, sdlVersionStruct.patch);
	sprintf_s(glewVersion, "%i.%i.%i", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
	sprintf_s(assimpVersion, "%i.%i.%i", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
	sprintf_s(devilVersion, "%i.%i.%i", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
	sprintf_s(mathgeolibVersion, "1.5"); // HARDCODED. Could not find MACRO.
	sprintf_s(imguiVersion, "%i.%i.%i", IMGUI_VERSION_NUM / 10000, (IMGUI_VERSION_NUM % 10000) / 100, IMGUI_VERSION_NUM % 100);
	//sprintf_s(imguizmoVersion, "");
	sprintf_s(fmtVersion, "%i.%i.%i", FMT_VERSION / 10000, (FMT_VERSION % 10000) / 100, FMT_VERSION % 100);
	sprintf_s(freetypeVersion, "%i.%i.%i", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
	sf_command(NULL, SFC_GET_LIB_VERSION, libsndVersion, sizeof(libsndVersion));
	sprintf_s(libsndVersion, &libsndVersion[11]); // Obtain the version numbers substring
	sprintf_s(physFSVersion, "%i.%i.%i", PHYSFS_VER_MAJOR, PHYSFS_VER_MINOR, PHYSFS_VER_PATCH);
	sprintf_s(rapidJsonVersion, "%i.%i.%i", RAPIDJSON_MAJOR_VERSION, RAPIDJSON_MINOR_VERSION, RAPIDJSON_PATCH_VERSION);
	sprintf_s(openALVersion, alGetString(AL_VERSION));
	sprintf_s(bulletVersion, "%i.%i", btGetVersion() / 100, (btGetVersion() % 100));
	sprintf_s(libavVersion, "%i.%i.%i", LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO);
	

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
