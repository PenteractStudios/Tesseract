#pragma once
#include "Panel.h"

class Component;
class GameObject;

class PanelResource : public Panel {
public:
	PanelResource();

	void Update() override;
};
