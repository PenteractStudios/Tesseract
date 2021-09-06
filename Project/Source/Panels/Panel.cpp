#include "Panel.h"

#include "imgui.h"

#include "Utils/Leaks.h"

Panel::Panel(const char* name, bool enabled)
	: name(name)
	, enabled(enabled) {}

void Panel::Update() {}
