#include "ModuleAudio.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/alErrors.h"
#include "Utils/alcErrors.h"

#include "AL/al.h"
#include "AL/alc.h"

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
	if (alcIsExtensionPresent(openALDevice, "ALC_ENUMERATE_ALL_EXT")) {
		name = alcGetString(openALDevice, ALC_ALL_DEVICES_SPECIFIER);
	}
	if (!name || alcGetError(openALDevice) != AL_NO_ERROR) {
		name = alcGetString(openALDevice, ALC_DEVICE_SPECIFIER);
	}
	LOG("Using Sound Device: \"%s\"", name);

	// Generate Sources
	alGenSources(NUM_SOURCES, sources);

	return true;
}

ALuint ModuleAudio::GetAvailableSource(bool reverse) const {
	if (reverse) {
		for (int i = NUM_SOURCES - 1; i >= 0; --i) {
			if (isAvailable(sources[i])) {
				return sources[i];
			}
		}
		return false;

	} else {
		for (int i = 0; i < NUM_SOURCES; ++i) {
			if (isAvailable(sources[i])) {
				return sources[i];
			}
		}
		return false;
	}
}

bool ModuleAudio::isActive(unsigned sourceId) const {
	ALint state;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING || state == AL_PAUSED);
}

bool ModuleAudio::isAvailable(unsigned sourceId) const {
	ALint state;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
	return (state == AL_STOPPED || state == AL_INITIAL);
}

void ModuleAudio::Stop(unsigned sourceId) const {
	if (sourceId) {
		alSourceStop(sourceId);
		alSourcei(sourceId, AL_BUFFER, NULL);
	}
}

void ModuleAudio::StopAllSources() {
	for (int i = 0; i < NUM_SOURCES; ++i) {
		if (isActive(sources[i])) {
			Stop(sources[i]);
		}
	}
}

bool ModuleAudio::CleanUp() {
	alCall(alDeleteSources, NUM_SOURCES, sources);
	alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
	alcCall(alcDestroyContext, openALDevice, openALContext);
	alcCloseDevice(openALDevice);

	return true;
}