#pragma once

#include "FileSystem/JsonValue.h"

#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>

#define BUFFER_LEN 4096

namespace AudioImporter {
	bool ImportAudio(const char* filePath, JsonValue jMeta);

	void EncondeWavToOgg(const char* infilename, const char* outfilename);
};
