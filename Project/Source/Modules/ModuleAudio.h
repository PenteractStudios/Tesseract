#pragma once

#include "Module.h"

#define NUM_SOURCES 32

#if defined(TESSERACT_ENGINE_API)
/* do nothing. */
#elif defined(_MSC_VER)
#define TESSERACT_ENGINE_API __declspec(dllexport)
#endif

struct ALCdevice;
struct ALCcontext;

class ModuleAudio : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;
	bool CleanUp() override;

	unsigned GetAvailableSource(bool reverse = false) const;
	bool isActive(unsigned sourceId) const;
	bool isAvailable(unsigned sourceId) const;
	void Stop(unsigned sourceID) const;
	TESSERACT_ENGINE_API void StopAllSources();

private:
	ALCdevice* openALDevice = nullptr;
	ALCcontext* openALContext = nullptr;
	bool contextMadeCurrent = false;
	unsigned sources[NUM_SOURCES] = {0};
};