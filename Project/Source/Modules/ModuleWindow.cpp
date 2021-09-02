#include "ModuleWindow.h"

#include "Application.h"
#include "Utils/Logging.h"
#include "Modules/ModuleEvents.h"
#include "Modules/ModuleScene.h"
#include "Resources/ResourceTexture.h"

#include "SDL.h"
#include "SDL_image.h"
#include "Scene.h"
#include "GL/glew.h"

#include "Utils/Leaks.h"

bool ModuleWindow::Init() {
	LOG("Init SDL window & surface");

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // desired version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

#if !GAME
	flags |= SDL_WINDOW_RESIZABLE;
#endif

	SDL_DisplayMode desktopDisplayMode;
	SDL_GetDesktopDisplayMode(0, &desktopDisplayMode);
	window = SDL_CreateWindow(App->appName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desktopDisplayMode.w - 100, desktopDisplayMode.h - 100, flags);
	if (window == NULL) {
		LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	surface = SDL_GetWindowSurface(window);

	// Create a list with all the available display modes
	int displayModeNum = SDL_GetNumDisplayModes(SDL_GetWindowDisplayIndex(window));
	for (int i = 0; i < displayModeNum; ++i) {
		SDL_DisplayMode displayMode;
		SDL_GetDisplayMode(SDL_GetWindowDisplayIndex(window), i, &displayMode);
		displayModes.push_back(displayMode);
		if (desktopDisplayMode.w == displayMode.w && desktopDisplayMode.h == displayMode.h && desktopDisplayMode.refresh_rate == displayMode.refresh_rate) {
			currentDisplayMode = i;
		}
	}

#if GAME
	SetWindowMode(WindowMode::FULLSCREEN);
	SetCursor(true);
#else
	SetWindowMode(WindowMode::WINDOWED);
#endif

	return true;
}

bool ModuleWindow::CleanUp() {
	LOG("Destroying SDL window and quitting all SDL systems");

	if (window != NULL) {
		SDL_DestroyWindow(window);
	}

	if (cursor != nullptr) {
		SDL_FreeCursor(cursor);
	}

	SDL_Quit();
	return true;
}

void ModuleWindow::SetWindowMode(WindowMode mode) {
	switch (mode) {
	case WindowMode::WINDOWED:
		SDL_SetWindowFullscreen(window, SDL_FALSE);
		ResetToDefaultSize();
		break;
	case WindowMode::FULLSCREEN:
		SetCurrentDisplayMode(currentDisplayMode);
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		break;
	case WindowMode::FULLSCREEN_DESKTOP:
		SetCurrentDisplayMode(currentDisplayMode);
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		break;
	}
	windowMode = mode;
}

void ModuleWindow::SetResizable(bool resizable) {
	if (resizable) {
		SDL_SetWindowResizable(window, SDL_TRUE);
	} else {
		SDL_SetWindowResizable(window, SDL_FALSE);
	}
}

void ModuleWindow::SetCurrentDisplayMode(unsigned index) {
	if (index >= displayModes.size()) return;
	const SDL_DisplayMode& displayMode = displayModes[index];
	SDL_SetWindowDisplayMode(window, &displayMode);
	currentDisplayMode = index;
	SetSize(displayMode.w, displayMode.h);
}

void ModuleWindow::SetSize(int width, int height) {
	int displayIndex = SDL_GetWindowDisplayIndex(window);
	SDL_SetWindowSize(window, width, height);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
	TesseractEvent resizeEvent(TesseractEventType::SCREEN_RESIZED);
	resizeEvent.Set<ViewportResizedStruct>(width, height);
	App->events->AddEvent(resizeEvent);
}

void ModuleWindow::ResetToDefaultSize() {
	SDL_DisplayMode desktopDisplayMode;
	SDL_GetDesktopDisplayMode(0, &desktopDisplayMode);
	SetSize(desktopDisplayMode.w - 100, desktopDisplayMode.h - 100);
}

void ModuleWindow::SetBrightness(float brightness) {
	SDL_SetWindowBrightness(window, brightness);
}

void ModuleWindow::SetTitle(const char* title) {
	SDL_SetWindowTitle(window, title);
}

void ModuleWindow::SetCursor(bool isPlaying) {
	if (isPlaying) {
		// Load resource
		Scene* scene = App->scene->scene;
		ResourceTexture* cursorResourceTexture = App->resources->GetResource<ResourceTexture>(scene->GetCursor());
		if (cursorResourceTexture == nullptr) {
			LOG("Error cursorResourceTexture null");
			return;
		}
		// From glTexture to SDL_Surface
		int w = 0;
		int h = 0;
		glGetTextureLevelParameteriv(cursorResourceTexture->glTexture, 0, GL_TEXTURE_WIDTH, &w);
		glGetTextureLevelParameteriv(cursorResourceTexture->glTexture, 0, GL_TEXTURE_HEIGHT, &h);

		void* pixel_data = malloc(w*h*4);
		memset(pixel_data, 0, w*h * 4);

		glBindTexture(GL_TEXTURE_2D, cursorResourceTexture->glTexture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixel_data, w, h, 32, 4 * w, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

		// Scale image
		SDL_Surface* scaledImage = SDL_CreateRGBSurfaceWithFormat(0, scene->widthCursor, scene->heightCursor, 32, SDL_PIXELFORMAT_RGBA32);
		SDL_SoftStretch(surface, &surface->clip_rect, scaledImage, &scaledImage->clip_rect);

		if (scaledImage) {
			cursor = SDL_CreateColorCursor(scaledImage, scene->widthCursor / 2, scene->heightCursor / 2);
		} else {
			LOG("Error creating Cursor");
			cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
		}
		free(pixel_data);
		pixel_data = nullptr;
	} else {
		cursor = SDL_GetDefaultCursor();
	}
	SDL_SetCursor(cursor);
}

WindowMode ModuleWindow::GetWindowMode() const {
	return windowMode;
}

bool ModuleWindow::GetMaximized() const {
	return (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) != 0;
}

bool ModuleWindow::GetResizable() const {
	return (SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE) != 0;
}

unsigned ModuleWindow::GetCurrentDisplayMode() const {
	return currentDisplayMode;
}

unsigned ModuleWindow::GetNumDisplayModes() const {
	return displayModes.size();
}

const SDL_DisplayMode& ModuleWindow::GetDisplayMode(unsigned index) const {
	return displayModes[index];
}

int ModuleWindow::GetWidth() const {
	int width;
	int height;
	SDL_GetWindowSize(window, &width, &height);
	return width;
}

int ModuleWindow::GetHeight() const {
	int width;
	int height;
	SDL_GetWindowSize(window, &width, &height);
	return height;
}

int ModuleWindow::GetPositionX() const {
	int posX, posY;
	SDL_GetWindowPosition(window, &posX, &posY);
	return posX;
}
int ModuleWindow::GetPositionY() const {
	int posX, posY;
	SDL_GetWindowPosition(window, &posX, &posY);
	return posY;
}

float ModuleWindow::GetBrightness() const {
	return SDL_GetWindowBrightness(window);
}

const char* ModuleWindow::GetTitle() const {
	return SDL_GetWindowTitle(window);
}

SDL_Cursor* ModuleWindow::GetCursor() const {
	return cursor;
}
