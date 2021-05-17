#pragma once

#include "Panel.h"

#include "imgui.h"
#include "ImGuizmo.h"

class PanelControlEditor : public Panel {
public:
	PanelControlEditor();

	void Update() override;

	// --- Control Bar Getters --- //
	ImGuizmo::OPERATION GetImGuizmoOperation() const;
	ImGuizmo::MODE GetImGuizmoMode() const;
	bool GetImGuizmoUseSnap() const;
	void GetImguizmoSnap(float* snap) const;

private:
	//ImGuizmo
	ImGuizmo::OPERATION currentGuizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentGuizmoMode = ImGuizmo::WORLD;

	bool pivotMode = false;
	bool useSnap = false;
	float snap[3] = {1.f, 1.f, 1.f};
};
