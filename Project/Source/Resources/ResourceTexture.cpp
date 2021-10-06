#include "ResourceTexture.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "Utils/MSTimer.h"
#include "Utils/FileUtils.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Logging.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"
#include "imgui.h"

#define JSON_TAG_COMPRESSION "Compression"
#define JSON_TAG_WRAP "Wrap"
#define JSON_TAG_MINFILTER "MinFilter"
#define JSON_TAG_MAGFILTER "MagFilter"

void ResourceTexture::Load() {
	std::string filePath = GetResourceFilePath();
	LOG("Loading texture from path: \"%s\".", filePath.c_str());

	// Timer to measure loading a texture
	MSTimer timer;
	timer.Start();

	// Load image
	unsigned width = 0;
	unsigned height = 0;
	unsigned bpp = 0;
	unsigned dataSize = 0;
	unsigned char* imageData = nullptr;
	DEFER {
		RELEASE(imageData);
	};

	switch (compression) {
	case TextureCompression::DXT1:
	case TextureCompression::DXT3:
	case TextureCompression::DXT5:
	case TextureCompression::BC7: {
		Buffer<char> buffer = App->files->Load(filePath.c_str());

		unsigned char* cursor = (unsigned char*) buffer.Data();
		DDSHeader* header = (DDSHeader*) cursor;
		cursor += sizeof(DDSHeader);

		width = header->width;
		height = header->height;
		bpp = header->pixelFormat.size / 8;
		dataSize = header->pitchOrLinearSize;

		imageData = new unsigned char[dataSize];
		memcpy(imageData, cursor, dataSize);
		break;
	}
	default: {
		// Generate image handler
		unsigned image;
		ilGenImages(1, &image);
		DEFER {
			ilDeleteImages(1, &image);
		};

		ilBindImage(image);
		bool imageLoaded = ilLoad(IL_TGA, filePath.c_str());
		if (!imageLoaded) {
			LOG("Failed to load image.");
			return;
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		bpp = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);

		ILenum format = bpp == 4 ? IL_RGBA : IL_RGB;

		// Convert image
		bool imageConverted = ilConvertImage(format, IL_UNSIGNED_BYTE);
		if (!imageConverted) {
			LOG("Failed to convert image.");
			return;
		}

		dataSize = width * height * bpp;
		imageData = new unsigned char[dataSize];
		memcpy(imageData, ilGetData(), dataSize);

		break;
	}
	}

	// Generate texture from image
	glGenTextures(1, &glTexture);
	glBindTexture(GL_TEXTURE_2D, glTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	switch (compression) {
	case TextureCompression::NONE: {
		int internalFormat = bpp == 4 ? GL_RGBA8 : GL_RGB8;
		int format = bpp == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
		break;
	}
	case TextureCompression::DXT1: {
		int internalFormat = bpp == 4 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataSize, imageData);
		break;
	}
	case TextureCompression::DXT3: {
		int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataSize, imageData);
		break;
	}
	case TextureCompression::DXT5: {
		int internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataSize, imageData);
		break;
	}
	case TextureCompression::BC7: {
		int internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_EXT;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataSize, imageData);
		break;
	}
	}

	// Generate mipmaps and set filtering and wrapping
	glGenerateMipmap(GL_TEXTURE_2D);
	UpdateWrap(wrap);
	UpdateMinFilter(minFilter);
	UpdateMagFilter(magFilter);

	unsigned timeMs = timer.Stop();
	LOG("Texture loaded in %ums.", timeMs);
}

void ResourceTexture::Unload() {
	if (glTexture) {
		glDeleteTextures(1, &glTexture);
		glTexture = 0;
	}
}

void ResourceTexture::LoadResourceMeta(JsonValue jResourceMeta) {
	compression = (TextureCompression)(int) jResourceMeta[JSON_TAG_COMPRESSION];
	wrap = (TextureWrap)(int) jResourceMeta[JSON_TAG_WRAP];
	minFilter = (TextureMinFilter)(int) jResourceMeta[JSON_TAG_MINFILTER];
	magFilter = (TextureMagFilter)(int) jResourceMeta[JSON_TAG_MAGFILTER];
}

void ResourceTexture::SaveResourceMeta(JsonValue jResourceMeta) {
	jResourceMeta[JSON_TAG_COMPRESSION] = (int) compression;
	jResourceMeta[JSON_TAG_WRAP] = (int) wrap;
	jResourceMeta[JSON_TAG_MINFILTER] = (int) minFilter;
	jResourceMeta[JSON_TAG_MAGFILTER] = (int) magFilter;
}

void ResourceTexture::UpdateMinFilter(TextureMinFilter filter) {
	glBindTexture(GL_TEXTURE_2D, glTexture);
	switch (filter) {
	case TextureMinFilter::NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case TextureMinFilter::LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case TextureMinFilter::NEAREST_MIPMAP_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		break;
	case TextureMinFilter::LINEAR_MIPMAP_NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		break;
	case TextureMinFilter::NEAREST_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		break;
	case TextureMinFilter::LINEAR_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	}
}

void ResourceTexture::UpdateMagFilter(TextureMagFilter filter) {
	glBindTexture(GL_TEXTURE_2D, glTexture);
	switch (filter) {
	case TextureMagFilter::NEAREST:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case TextureMagFilter::LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
}

void ResourceTexture::UpdateWrap(TextureWrap textureWrap) {
	glBindTexture(GL_TEXTURE_2D, glTexture);
	switch (textureWrap) {
	case TextureWrap::REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case TextureWrap::CLAMP_TO_EDGE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case TextureWrap::CLAMP_TO_BORDER:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		break;
	case TextureWrap::MIRROR_REPEAT:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	case TextureWrap::MIRROR_CLAMP_TO_EDGE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
		break;
	}
}