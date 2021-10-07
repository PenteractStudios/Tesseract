#pragma once

#include "Module.h"

#include "AL/alc.h"

#include <vector>
#include <string>

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
	UpdateStatus Update() override;
	bool CleanUp() override;

	bool OpenSoundDevice(ALCchar* device = nullptr);
	bool CloseSoundDevice();

	void GetSoundDevices(std::vector<std::string>& devicesParsed);
	const std::string GetCurrentDevice();
	void SetSoundDevice(int pos);

	unsigned GetAvailableSource(bool reverse = false) const;
	bool isActive(unsigned sourceId) const;
	bool isAvailable(unsigned sourceId) const;
	void Stop(unsigned sourceID) const;
	TESSERACT_ENGINE_API void StopAllSources();

private:
	std::vector<ALCchar*> devices;
	ALCchar* currentDevice;
	ALCdevice* openALDevice = nullptr;
	ALCcontext* openALContext = nullptr;
	bool contextMadeCurrent = false;
	unsigned sources[NUM_SOURCES] = {0};
};