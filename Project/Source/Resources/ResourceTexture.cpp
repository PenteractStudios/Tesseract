#include "ResourceTexture.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "Utils/MSTimer.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "Utils/Logging.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "GL/glew.h"
#include "imgui.h"

#define JSON_TAG_MINFILTER "MinFilter"
#define JSON_TAG_MAGFILTER "MagFilter"
#define JSON_TAG_WRAP "Wrap"

void ResourceTexture::Load() {
	// Timer to measure loading a texture
	MSTimer timer;
	timer.Start();
	std::string filePath = GetResourceFilePath();
	LOG("Loading texture from path: \"%s\".", filePath.c_str());

	// Generate image handler
	unsigned image;
	ilGenImages(1, &image);
	DEFER {
		ilDeleteImages(1, &image);
	};

	// Load image
	ilBindImage(image);
	bool imageLoaded = ilLoad(IL_DDS, filePath.c_str());
	if (!imageLoaded) {
		LOG("Failed to load image.");
		return;
	}

	// Flip image if neccessary
	ILinfo info;
	iluGetImageInfo(&info);
	if (info.Origin == IL_ORIGIN_UPPER_LEFT) {
		iluFlipImage();
	}

	// Generate texture from image
	glGenTextures(1, &glTexture);
	glBindTexture(GL_TEXTURE_2D, glTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	// Generate mipmaps and set filtering and wrapping
	glGenerateMipmap(GL_TEXTURE_2D);
	UpdateWrap(wrap);
	UpdateMinFilter(minFilter);
	UpdateMagFilter(magFilter);

	unsigned timeMs = timer.Stop();
	LOG("Texture loaded in %ums.", timeMs);
}

void ResourceTexture::Unload() {
	glDeleteTextures(1, &glTexture);
}

void ResourceTexture::OnEditorUpdate() {
	ImGui::TextColored(App->editor->titleColor, "Filters");

	// Min filter combo box
	const char* min_filter_items[] = {"Nearest", "Linear", "Nearest Mipmap Nearest", "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear"};
	const char* min_filter_item_current = min_filter_items[int(minFilter)];
	if (ImGui::BeginCombo("Min filter", min_filter_item_current)) {
		for (int n = 0; n < IM_ARRAYSIZE(min_filter_items); ++n) {
			bool is_selected = (min_filter_item_current == min_filter_items[n]);
			if (ImGui::Selectable(min_filter_items[n], is_selected)) {
				minFilter = TextureMinFilter(n);
				hasChanged = true;
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
				hasChanged = true;
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
				hasChanged = true;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	if (hasChanged) {
		if (ImGui::Button("Apply")) {
			Apply();
			hasChanged = false;
		}
	}
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

void ResourceTexture::UpdateWrap(TextureWrap wrap) {
	glBindTexture(GL_TEXTURE_2D, glTexture);
	switch (wrap) {
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

void ResourceTexture::Apply() {
	UpdateWrap(wrap);
	UpdateMinFilter(minFilter);
	UpdateMagFilter(magFilter);
}