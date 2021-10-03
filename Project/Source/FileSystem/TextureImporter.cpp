#include "TextureImporter.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleEditor.h"
#include "Resources/ResourceTexture.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/MSTimer.h"
#include "Utils/FileDialog.h"
#include "Utils/FileUtils.h"
#include "ImporterCommon.h"

#include "compressonator.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"
#include "rapidjson/prettywriter.h"

#include "Utils/Leaks.h"

#define JSON_TAG_FLIP "Flip"
#define JSON_TAG_COMPRESSION "Compression"
#define JSON_TAG_WRAP "Wrap"
#define JSON_TAG_MIN_FILTER "MinFilter"
#define JSON_TAG_MAG_FILTER "MagFilter"

Buffer<unsigned char> CompressTexture(TextureCompression compression, int width, int height, const unsigned char* data) {
	Buffer<unsigned char> compressedData;

	if (compression == TextureCompression::NONE) {
		compressedData.Allocate(width * height * 4);
		memcpy(compressedData.Data(), data, compressedData.Size());
		return compressedData;
	}

	CMP_Texture source = {0};
	source.dwSize = sizeof(CMP_Texture);
	source.dwWidth = width;
	source.dwHeight = height;
	source.dwPitch = width * 4;
	source.format = CMP_FORMAT_RGBA_8888;
	source.dwDataSize = width * height * 4;
	source.pData = (CMP_BYTE*) data;

	CMP_Texture destination = {0};
	destination.dwSize = sizeof(CMP_Texture);
	destination.dwWidth = width;
	destination.dwHeight = height;
	destination.dwPitch = width;
	switch (compression) {
	case TextureCompression::DXT1:
		destination.format = CMP_FORMAT_BC1;
		break;
	case TextureCompression::DXT3:
		destination.format = CMP_FORMAT_BC2;
		break;
	case TextureCompression::DXT5:
		destination.format = CMP_FORMAT_BC3;
		break;
	case TextureCompression::BC7:
		destination.format = CMP_FORMAT_BC7;
		break;
	}
	destination.dwDataSize = CMP_CalculateBufferSize(&destination);
	compressedData.Allocate(destination.dwDataSize);
	destination.pData = compressedData.Data();

	CMP_CompressOptions options = {0};
	options.dwSize = sizeof(CMP_CompressOptions);
	options.fquality = 1.0f;

	CMP_ERROR error = CMP_ConvertTexture(&source, &destination, &options, nullptr);
	if (error != CMP_OK) {
		LOG("Error converting texture: %i", error);
		return Buffer<unsigned char>();
	}

	return compressedData;
}

void TextureImportOptions::ShowImportOptions() {
	// Flip
	ImGui::Checkbox("Flip", &flip);

	// Compression combo box
	const char* compression_items[] = {"None", "DXT1", "DXT3", "DXT5", "BC7"};
	const char* compression_item_current = compression_items[int(compression)];
	if (ImGui::BeginCombo("Compression", compression_item_current)) {
		for (int n = 0; n < IM_ARRAYSIZE(compression_items); ++n) {
			bool is_selected = (compression_item_current == compression_items[n]);
			if (ImGui::Selectable(compression_items[n], is_selected)) {
				compression = TextureCompression(n);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Filters
	ImGui::TextColored(App->editor->titleColor, "Filters");

	// Min filter combo box
	const char* min_filter_items[] = {"Nearest", "Linear", "Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear"};
	const char* min_filter_item_current = min_filter_items[int(minFilter)];
	if (ImGui::BeginCombo("Min filter", min_filter_item_current)) {
		for (int n = 0; n < IM_ARRAYSIZE(min_filter_items); ++n) {
			bool is_selected = (min_filter_item_current == min_filter_items[n]);
			if (ImGui::Selectable(min_filter_items[n], is_selected)) {
				minFilter = TextureMinFilter(n);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Mag filter combo box
	const char* mag_filter_items[] = {"Nearest", "Linear"};
	const char* mag_filter_item_current = mag_filter_items[int(magFilter)];
	if (ImGui::BeginCombo("Mag filter", mag_filter_item_current)) {
		for (int n = 0; n < IM_ARRAYSIZE(mag_filter_items); ++n) {
			bool is_selected = (mag_filter_item_current == mag_filter_items[n]);
			if (ImGui::Selectable(mag_filter_items[n], is_selected)) {
				magFilter = TextureMagFilter(n);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Texture wrap combo box
	const char* wrap_items[] = {"Repeat", "Clamp to Edge", "Clamp to Border", "Mirrored Repeat", "Mirrored Clamp to Edge"};
	const char* wrap_item_current = wrap_items[int(wrap)];
	if (ImGui::BeginCombo("Wrap", wrap_item_current)) {
		for (int n = 0; n < IM_ARRAYSIZE(wrap_items); ++n) {
			bool is_selected = (wrap_item_current == wrap_items[n]);
			if (ImGui::Selectable(wrap_items[n], is_selected)) {
				wrap = TextureWrap(n);
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void TextureImportOptions::Load(JsonValue jMeta) {
	flip = jMeta[JSON_TAG_FLIP];
	compression = (TextureCompression)(int) jMeta[JSON_TAG_COMPRESSION];
	wrap = (TextureWrap)(int) jMeta[JSON_TAG_WRAP];
	minFilter = (TextureMinFilter)(int) jMeta[JSON_TAG_MIN_FILTER];
	magFilter = (TextureMagFilter)(int) jMeta[JSON_TAG_MAG_FILTER];
}

void TextureImportOptions::Save(JsonValue jMeta) {
	jMeta[JSON_TAG_FLIP] = flip;
	jMeta[JSON_TAG_COMPRESSION] = (int) compression;
	jMeta[JSON_TAG_WRAP] = (int) wrap;
	jMeta[JSON_TAG_MIN_FILTER] = (int) minFilter;
	jMeta[JSON_TAG_MAG_FILTER] = (int) magFilter;
}

bool TextureImporter::ImportTexture(const char* filePath, JsonValue jMeta) {
	LOG("Importing texture from path: \"%s\".", filePath);

	// Timer to measure importing a texture
	MSTimer timer;
	timer.Start();

	// Load import options
	TextureImportOptions* importOptions = App->resources->GetImportOptions<TextureImportOptions>(filePath);
	if (!importOptions) {
		LOG("Failed to load import options.");
		return false;
	}

	// Generate image handler
	unsigned image;
	ilGenImages(1, &image);
	DEFER {
		ilDeleteImages(1, &image);
	};

	// Load image
	ilBindImage(image);
	bool imageLoaded = ilLoadImage(filePath);
	if (!imageLoaded) {
		LOG("Failed to load image.");
		return false;
	}

	int width = ilGetInteger(IL_IMAGE_WIDTH);
	int height = ilGetInteger(IL_IMAGE_HEIGHT);

	// Convert image
	bool imageConverted = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!imageConverted) {
		LOG("Failed to convert image.");
		return false;
	}

	// Flip if asked to
	if (importOptions->flip) {
		iluFlipImage();
	}

	// Create texture resource
	unsigned resourceIndex = 0;
	std::unique_ptr<ResourceTexture> texture = ImporterCommon::CreateResource<ResourceTexture>(FileDialog::GetFileName(filePath).c_str(), filePath, jMeta, resourceIndex);

	texture->compression = importOptions->compression;
	texture->wrap = importOptions->wrap;
	texture->minFilter = importOptions->minFilter;
	texture->magFilter = importOptions->magFilter;

	// Save import options to the meta file
	importOptions->Save(jMeta);

	// Save resource meta file
	bool saved = ImporterCommon::SaveResourceMetaFile(texture.get());
	if (!saved) {
		LOG("Failed to save texture resource meta file.");
		return false;
	}

	// Compress image
	Buffer<char> buffer;
	switch (importOptions->compression) {
	case TextureCompression::DXT1:
	case TextureCompression::DXT3: 
	case TextureCompression::DXT5:
	case TextureCompression::BC7: {
		iluFlipImage();

		Buffer<unsigned char> compressedData = CompressTexture(importOptions->compression, width, height, ilGetData());

		buffer.Allocate(sizeof(DDSHeader) + compressedData.Size());

		char* cursor = buffer.Data();
		DDSHeader* header = (DDSHeader*) cursor;
		cursor += sizeof(DDSHeader);

		memset(header, 0, sizeof(DDSHeader));
		header->magic = ('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24);
		header->size = 124;
		header->flags = DDSHeader::CAPS | DDSHeader::HEIGHT | DDSHeader::WIDTH | DDSHeader::PIXELFORMAT | DDSHeader::LINEARSIZE;
		header->width = width;
		header->height = height;
		header->pitchOrLinearSize = compressedData.Size();
		header->pixelFormat.size = 32;
		header->pixelFormat.flags = DDSHeader::PixelFormat::FOURCC;
		switch (importOptions->compression) {
		case TextureCompression::DXT1:
			header->pixelFormat.fourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('1' << 24);
			break;
		case TextureCompression::DXT3:
			header->pixelFormat.fourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('3' << 24);
			break;
		case TextureCompression::DXT5:
			header->pixelFormat.fourCC = ('D' << 0) | ('X' << 8) | ('T' << 16) | ('5' << 24);
			break;
		case TextureCompression::BC7:
			header->pixelFormat.fourCC = ('B' << 0) | ('C' << 8) | ('7' << 16) | (' ' << 24);
			break;
		}
		header->caps.caps1 = DDSHeader::Caps::TEXTURE;

		memcpy(cursor, compressedData.Data(), compressedData.Size());
		break;
	}
	default: {
		unsigned size = ilSaveL(IL_TGA, nullptr, 0);
		if (size == 0) {
			LOG("Failed to save image.");
			return false;
		};
		buffer.Allocate(size);
		size = ilSaveL(IL_TGA, buffer.Data(), size);
		if (size == 0) {
			LOG("Failed to save image.");
			return false;
		}
		break;
	}
	}

	// Save to file
	saved = App->files->Save(texture->GetResourceFilePath().c_str(), buffer);
	if (!saved) {
		LOG("Failed to save texture resource file.");
		return false;
	}

	// Send resource creation event
	App->resources->SendCreateResourceEvent(texture);

	unsigned timeMs = timer.Stop();
	LOG("Texture imported in %ums", timeMs);
	return true;
}
