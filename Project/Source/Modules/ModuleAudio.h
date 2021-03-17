#pragma once
#include "Module.h"

#include "AL/alc.h"
#include <vector>
#include <string>

class ModuleAudio : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;
	bool CleanUp() override;

private:
	ALCdevice* openALDevice = nullptr;
	ALCcontext* openALContext = nullptr;
	ALCboolean contextMadeCurrent = false;
};