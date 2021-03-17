#include "ModuleAudio.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/alcErrors.h"
#include "AL/al.h"

#include "Utils/Leaks.h"

bool ModuleAudio::Init() {
	openALDevice = alcOpenDevice(nullptr); // Get Sound Device. nullptr = default
	if (!openALDevice) {
		LOG("ERROR: Failed to get sound device");
		return false;
	}

	if (!alcCall(alcCreateContext, openALContext, openALDevice, openALDevice, nullptr) || !openALContext) { // Create Context
		LOG("ERROR: Could not create audio context");
		return false;
	}

	if (!alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, openALContext) // Make Context Current
		|| contextMadeCurrent != ALC_TRUE) {
		LOG("ERROR: Could not make audio context current");
		return false;
	}

	const ALCchar* name = nullptr;
	if (alcIsExtensionPresent(openALDevice, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(openALDevice, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(openALDevice) != AL_NO_ERROR)
		name = alcGetString(openALDevice, ALC_DEVICE_SPECIFIER);
	LOG("Using Sound Device: \"%s\"", name);

	return true;
}

bool ModuleAudio::CleanUp() {
	alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
	alcCall(alcDestroyContext, openALDevice, openALContext);
	alcCloseDevice(openALDevice);

	return true;
}