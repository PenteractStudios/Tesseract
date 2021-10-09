#include "AudioImporter.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Resources/ResourceAudioClip.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/FileDialog.h"
#include "ImporterCommon.h"

#include <sndfile.h>

#include "Utils/Leaks.h"

#define JSON_TAG_IS_MONO "IsMono"
#define JSON_TAG_AUDIO_FORMAT "AudioFormat"

void AudioImportOptions::ShowImportOptions() {
	ImGui::PushItemWidth(150);
	ImGui::TextColored(App->editor->titleColor, "Audio Options");
	ImGui::NewLine();
	ImGui::Checkbox("Force To Mono", &isMono);
	ImGui::NewLine();
	const char* audioFormatItems[] = {"PCM (.wav)", "Vorbis (.ogg) (Not working)"};
	const char* currentAudioFormatItems = audioFormatItems[int(audioFormat)];
	if (ImGui::BeginCombo("Compression Format", currentAudioFormatItems)) {
		for (int n = 0; n < IM_ARRAYSIZE(audioFormatItems); ++n) {
			bool is_selected = (currentAudioFormatItems == audioFormatItems[n]);
			if (ImGui::Selectable(audioFormatItems[n], is_selected)) {
				audioFormat = AudioFormat(n);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void AudioImportOptions::Load(JsonValue jMeta) {
	isMono = jMeta[JSON_TAG_IS_MONO];
	audioFormat = (AudioFormat)(int) jMeta[JSON_TAG_AUDIO_FORMAT];
}

void AudioImportOptions::Save(JsonValue jMeta) {
	jMeta[JSON_TAG_IS_MONO] = isMono;
	jMeta[JSON_TAG_AUDIO_FORMAT] = (int) audioFormat;
}

bool AudioImporter::ImportAudio(const char* filePath, JsonValue jMeta) {
	LOG("Importing audio from path: \"%s\".", filePath);

	// Timer to measure importing audio
	MSTimer timer;
	timer.Start();

	// Load import options
	AudioImportOptions* importOptions = App->resources->GetImportOptions<AudioImportOptions>(filePath);
	if (!importOptions) {
		LOG("Failed to load import options.");
		return false;
	}

	// Save import options to the meta file
	importOptions->Save(jMeta);

	// Convert & save audio
	std::string fileName = FileDialog::GetFileNameAndExtension(filePath);
	std::string fileNameTemp = "temp_" + fileName;
	std::string fileIn(filePath);
	const char* tempFilePath = fileIn.replace(fileIn.end() - fileName.length(), fileIn.end(), fileNameTemp).c_str();

	if (!ConvertAudio(filePath, tempFilePath, importOptions)) {
		return false;
	}
	DEFER {
		App->files->Erase(tempFilePath);
	};
	Buffer<char> buffer = App->files->Load(tempFilePath);
	if (buffer.Size() == 0) {
		LOG("Error loading audio %s", tempFilePath);
		return false;
	}

	// Create audio clip resource
	unsigned resourceIndex = 0;
	std::unique_ptr<ResourceAudioClip> audioClip = ImporterCommon::CreateResource<ResourceAudioClip>(FileDialog::GetFileName(filePath).c_str(), filePath, jMeta, resourceIndex);

	audioClip->isMono = importOptions->isMono;
	audioClip->audioFormat = importOptions->audioFormat;

	// Save resource meta file
	bool saved = ImporterCommon::SaveResourceMetaFile(audioClip.get());
	if (!saved) {
		LOG("Failed to save audio clip resource meta file.");
		return false;
	}

	// Save to file
	saved = App->files->Save(audioClip->GetResourceFilePath().c_str(), buffer);
	if (!saved) {
		LOG("Failed to save audio clip resource file.");
		return false;
	}

	// Send resource creation event
	App->resources->SendCreateResourceEvent(audioClip);

	unsigned timeMs = timer.Stop();
	LOG("Audio imported in %ums", timeMs);
	return true;
}

bool AudioImporter::ConvertAudio(const char* filePath, const char* convertedFilePath, const AudioImportOptions* audioImportOptions) {
	SNDFILE *inFile, *outFile;
	SF_INFO sfInfo;
	short *audioDataIn, *audioDataOut;

	// Open Read file
	if (!(inFile = sf_open(filePath, SFM_READ, &sfInfo))) {
		LOG("Error : could not open file %s", filePath);
		return false;
	}
	DEFER {
		sf_close(inFile);
	};

	// Input data
	audioDataIn = static_cast<short*>(malloc((size_t)(sfInfo.frames * sfInfo.channels) * sizeof(short)));
	DEFER {
		free(audioDataIn);
	};

	sf_count_t numFrames = sf_readf_short(inFile, audioDataIn, sfInfo.frames);

	// Output data
	audioDataOut = static_cast<short*>(malloc((size_t)(sfInfo.frames) * sizeof(short)));
	DEFER {
		free(audioDataOut);
	};

	// Mono conversion
	bool convertToMono = (sfInfo.channels == 2 && audioImportOptions->isMono);
	if (convertToMono) {
		// mixdown
		int channels = (int) sfInfo.channels;
		for (int i = 0; i < sfInfo.frames; ++i) {
			float data = 0;
			for (int j = 0; j < channels; j++)
				data += audioDataIn[i * channels + j];
			audioDataOut[i] = (short) (data / channels);
		}
		sfInfo.channels = 1;
	}

	// Format conversion
	if (audioImportOptions->audioFormat == AudioFormat::OGG) {
		//sfInfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
		sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
	} else {
		sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
	}
	if (!sf_format_check(&sfInfo)) {
		LOG("Invalid encoding format");
		return false;
	};

	// Open write file
	if (!(outFile = sf_open(convertedFilePath, SFM_WRITE, &sfInfo))) {
		LOG("Error : could not open file %s", convertedFilePath);
		return false;
	};
	DEFER {
		sf_close(outFile);
	};

	sfInfo.frames = numFrames;
	if (convertToMono) {
		sf_count_t numFramess = sf_writef_short(outFile, audioDataOut, sfInfo.frames);
	} else {
		sf_count_t numFramess = sf_writef_short(outFile, audioDataIn, sfInfo.frames);
	}

	return true;
}
