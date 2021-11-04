#include "ModuleAudio.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleScene.h"
#include "Utils/alErrors.h"
#include "Utils/alcErrors.h"
#include "Scene.h"

#include "AL/al.h"

#include "Utils/Leaks.h"

bool ModuleAudio::Init() {
	return OpenSoundDevice();
}

UpdateStatus ModuleAudio::Update() {
	size_t listeners = App->scene->scene->audioListenerComponents.Count();
	if (listeners <= 0) {
		LOG("Warning: Missing audio listener in scene");
	} else if (listeners > 1) {
		LOG("Warning: More than one audio listener in scene");
	}
	return UpdateStatus::CONTINUE;
}

bool ModuleAudio::CleanUp() {
	return CloseSoundDevice();
}

bool ModuleAudio::OpenSoundDevice(ALCchar* device) {
	openALDevice = alcOpenDevice(device); // Get Sound Device. nullptr = default
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
	alCall(alGenSources, NUM_SOURCES, sources);
	return true;
}

bool ModuleAudio::CloseSoundDevice() {
	StopAllSources();
	alCall(alDeleteSources, NUM_SOURCES, sources);
	memset(sources, 0, sizeof(sources));
	alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
	alcCall(alcDestroyContext, openALDevice, openALContext);
	alcCloseDevice(openALDevice);

	return true;
}

void ModuleAudio::GetSoundDevices(std::vector<std::string>& devicesParsed) {
	devices.clear();
	devicesParsed.clear();
	ALCchar* devicesList = (ALCchar*) alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	ALCchar* nptr;

	nptr = devicesList;
	while (*(nptr += strlen(devicesList) + 1) != 0) {
		devices.push_back(devicesList);
		devicesList = nptr;
	}
	devices.push_back(devicesList);

	for (std::string device : devices) {
		std::string newDevice = device.substr(15, device.length());
		devicesParsed.push_back(newDevice.c_str());
	}
}

const std::string ModuleAudio::GetCurrentDevice() {
	std::string currentDevice = alcGetString(openALDevice, ALC_ALL_DEVICES_SPECIFIER);
	return currentDevice.substr(15, currentDevice.length());
}

void ModuleAudio::SetSoundDevice(int pos) {
	CloseSoundDevice();
	OpenSoundDevice(devices[pos]);
}

unsigned ModuleAudio::GetAvailableSource(bool reverse) const {
	if (reverse) {
		for (int i = NUM_SOURCES - 1; i >= 0; --i) {
			if (!isActive(sources[i])) {
				return sources[i];
			}
		}
		return 0;

	} else {
		for (int i = 0; i < NUM_SOURCES; ++i) {
			if (!isActive(sources[i])) {
				return sources[i];
			}
		}
		return 0;
	}
}

void ModuleAudio::DeleteRelatedBuffer(unsigned int bufferId) {
	for (int i = 0; i < NUM_SOURCES; ++i) {
		ALint currentBuffer = 0;
		alGetSourcei(sources[i], AL_BUFFER, &currentBuffer);
		if (bufferId == currentBuffer) {
			alSourcei(sources[i], AL_BUFFER, NULL);
		}
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
		sourceId = 0;
	}
}

void ModuleAudio::StopAllSources() {
	for (int i = 0; i < NUM_SOURCES; ++i) {
		if (isActive(sources[i])) {
			Stop(sources[i]);
		}
	}
}

float ModuleAudio::GetGainMainChannel() {
	return gainMainChannel;
}

float ModuleAudio::GetGainMusicChannel() const {
	return gainMusicChannel;
}

float ModuleAudio::GetGainSFXChannel() const {
	return gainSFXChannel;
}

void ModuleAudio::SetGainMainChannel(float _gainMainChannel) {
	if (App->scene->scene->audioListenerComponents.Count() == 0) {
		return;
	}
	gainMainChannel = _gainMainChannel;
	Pool<ComponentAudioListener>::Iterator audioListener = App->scene->scene->audioListenerComponents.begin();
	(*audioListener).SetAudioVolume(gainMainChannel);
}

void ModuleAudio::SetGainMusicChannel(float _gainMusicChannel) {
	gainMusicChannel = _gainMusicChannel;
	for (ComponentAudioSource& audioSource : App->scene->scene->audioSourceComponents) {
		if (audioSource.GetIsMusic()) {
			audioSource.SetGainMultiplier(gainMusicChannel);
		}
	}
}

void ModuleAudio::SetGainSFXChannel(float _gainSFXChannel) {
	gainSFXChannel = _gainSFXChannel;
	for (ComponentAudioSource& audioSource : App->scene->scene->audioSourceComponents) {
		if (!audioSource.GetIsMusic()) {
			audioSource.SetGainMultiplier(gainSFXChannel);
		}
	}
}

void ModuleAudio::SetGainMainChannelInternal(float _gainMainChannel) {
	gainMainChannel = _gainMainChannel;
}

void ModuleAudio::SetGainMusicChannelInternal(float _gainMusicChannel) {
	gainMusicChannel = _gainMusicChannel;
}

void ModuleAudio::SetGainSFXChannelInternal(float _gainSFXChannel) {
	gainSFXChannel = _gainSFXChannel;
}
