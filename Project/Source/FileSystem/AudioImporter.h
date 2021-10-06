#pragma once

#include "FileSystem/JsonValue.h"
#include "FileSystem/ImportOptions.h"
#include "Resources/ResourceAudioClip.h"

#define BUFFER_LEN 4096

class AudioImportOptions : public ImportOptions {
public:
	void ShowImportOptions() override;
	void Load(JsonValue jMeta) override;
	void Save(JsonValue jMeta) override;

public:
	bool isMono = false;
	AudioFormat audioFormat = AudioFormat::WAV;
};

namespace AudioImporter {
	bool ImportAudio(const char* filePath, JsonValue jMeta);

	bool ConvertAudio(const char* filePath, const char* convertedFilePath, const AudioImportOptions* audioImportOptions);
}; // namespace AudioImporter
