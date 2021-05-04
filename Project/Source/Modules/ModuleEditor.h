#pragma once

#include "Module.h"

#include "Panels/PanelScene.h"
#include "Panels/PanelProject.h"
#include "Panels/PanelConsole.h"
#include "Panels/PanelConfiguration.h"
#include "Panels/PanelInspector.h"
#include "Panels/PanelHierarchy.h"
#include "Panels/PanelAbout.h"
#include "Panels/PanelControlEditor.h"
#include "Panels/PanelNavigation.h"

#include "imgui.h"
#include <vector>
#include <string>

enum class Modal {
	NONE,
	NEW_PROJECT,
	NEW_SCENE,
	LOAD_PROJECT,
	LOAD_SCENE,
	SAVE_PROJECT,
	SAVE_SCENE,
	COMPONENT_EXISTS,
	CREATE_MATERIAL,
	CREATE_SCRIPT,
	CANT_REMOVE_COMPONENT,
	QUIT
};

class ModuleEditor : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;
	bool Start() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;
	bool CleanUp() override;
	void OnMouseClicked();
	void OnMouseReleased();

public:
	Modal modalToOpen = Modal::NONE; // Used in the MenuBar to popup a Modal Window of the specific type.
	// ---------- Docking ----------
	unsigned dockMainId = 0;
	unsigned dockUpId = 0;
	unsigned dockLeftId = 0;
	unsigned dockRightId = 0;
	unsigned dockDownId = 0;

	// ---------- Panels ----------- //
	std::vector<Panel*> panels;
	PanelScene panelScene;
	PanelProject panelProject;
	PanelConsole panelConsole;
	PanelConfiguration panelConfiguration;
	PanelInspector panelInspector;
	PanelHierarchy panelHierarchy;
	PanelAbout panelAbout;
	PanelControlEditor panelControlEditor;
	PanelNavigation panelNavigation;

	GameObject* selectedGameObject = nullptr;			   // Pointer to the GameObject that will be shown in the inspector.
	ImVec4 titleColor = ImVec4(0.35f, 0.69f, 0.87f, 1.0f); // Color used for the titles in ImGui
	ImVec4 textColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);	   // Color used for text and information in ImGui

	// --- Float Sliders speeds ---- //
	float dragSpeed1f = 0.5f;
	float dragSpeed2f = 0.05f;
	float dragSpeed3f = 0.005f;
	float dragSpeed5f = 0.00005f;

private:
	std::string selectedFile = "";
};
