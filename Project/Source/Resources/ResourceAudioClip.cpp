#include "ResourceAudioClip.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleAudio.h"
#include "Utils/MSTimer.h"
#include "Utils/Logging.h"

#include "AL/alext.h"
#include <inttypes.h>
#include <sndfile.h>

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Leaks.h"

#define JSON_TAG_IS_MONO "IsMono"
#define JSON_TAG_AUDIO_FORMAT "AudioFormat"

void ResourceAudioClip::Load() {
	MSTimer timer;
	timer.Start();
	std::string filePath = GetResourceFilePath();
	LOG("Loading audio from path: \"%s\".", filePath.c_str());

	ALenum err, format;
	SNDFILE* sndfile;
	SF_INFO sfinfo;
	short* audioData;
	sf_count_t numFrames;
	ALsizei size;

	// Open Audio File
	sndfile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
	if (!sndfile) {
		LOG("Could not open audio in %s: %s", filePath.c_str(), sf_strerror(sndfile));
		return;
	}
	DEFER {
		sf_close(sndfile);
	};

	if (sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX / sizeof(short)) / sfinfo.channels) {
		LOG("Bad sample count in %s (%" PRId64 ")", filePath, sfinfo.frames);
		return;
	}

	format = AL_NONE;
	if (sfinfo.channels == 1) {
		format = AL_FORMAT_MONO16;
	} else if (sfinfo.channels == 2) {
		format = AL_FORMAT_STEREO16;
	}
	if (!format) {
		LOG("Unsupported channel count: %d", sfinfo.channels);
		return;
	}

	// Decode the whole audio file to a buffer
	audioData = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));
	DEFER {
		free(audioData);
	};
	numFrames = sf_readf_short(sndfile, audioData, sfinfo.frames);
	if (numFrames < 1) {
		LOG("Failed to read samples in %s (%" PRId64 ")", filePath, numFrames);
		return;
	}
	size = (ALsizei)(numFrames * sfinfo.channels) * (ALsizei) sizeof(short);

	ALbuffer = 0;
	alGenBuffers(1, &ALbuffer);
	alBufferData(ALbuffer, format, audioData, size, sfinfo.samplerate);

	// Check if an error occured, and clean up if so.
	err = alGetError();
	if (err != AL_NO_ERROR) {
		LOG("OpenAL Error: %s", alGetString(err));
		if (ALbuffer && alIsBuffer(ALbuffer)) {
			alDeleteBuffers(1, &ALbuffer);
		}
		return;
	}
}

void ResourceAudioClip::Unload() {
	if (ALbuffer) {
		alDeleteBuffers(1, &ALbuffer);
		ALbuffer = 0;
	}
}

void ResourceAudioClip::LoadResourceMeta(JsonValue jResourceMeta) {
	isMono = jResourceMeta[JSON_TAG_IS_MONO];
	audioFormat = (AudioFormat)(int) jResourceMeta[JSON_TAG_AUDIO_FORMAT];
}

void ResourceAudioClip::SaveResourceMeta(JsonValue jResourceMeta) {
	jResourceMeta[JSON_TAG_IS_MONO] = isMono;
	jResourceMeta[JSON_TAG_AUDIO_FORMAT] = (int) audioFormat;
}
