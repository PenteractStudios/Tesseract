#pragma once

#include "Globals.h"
#include "Module.h"

#include "SDL_video.h"
#include <vector>
#include <string>

struct SDL_Cursor;
struct SDL_Surface;

enum class WindowMode {
	WINDOWED,
	FULLSCREEN,
	FULLSCREEN_DESKTOP
};

class ModuleWindow : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;
	bool CleanUp() override;

	// ---------- Setters ---------- //
	void SetWindowMode(WindowMode mode);
	void SetResizable(bool resizable);
	void SetCurrentDisplayMode(unsigned index);
	void SetSize(int width, int height);
	void ResetToDefaultSize();
	void SetBrightness(float brightness);
	void SetTitle(const char* title);
	void SetCursor(bool isPlaying);

	// ---------- Getters ---------- //
	WindowMode GetWindowMode() const;
	bool GetMaximized() const;
	bool GetResizable() const;
	unsigned GetCurrentDisplayMode() const;
	unsigned GetNumDisplayModes() const;
	const SDL_DisplayMode& GetDisplayMode(unsigned index) const;
	int GetWidth() const;
	int GetHeight() const;
	int GetPositionX() const;
	int GetPositionY() const;
	float GetBrightness() const;
	const char* GetTitle() const;
	SDL_Cursor* GetCursor() const;

public:
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;
	SDL_Cursor* cursor = nullptr;

private:
	WindowMode windowMode = WindowMode::WINDOWED;
	std::vector<SDL_DisplayMode> displayModes;
	unsigned currentDisplayMode = 0;
};
