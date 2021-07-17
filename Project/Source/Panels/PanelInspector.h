#pragma once

#include "Panel.h"

class Component;
class GameObject;

class PanelInspector : public Panel {
public:
	PanelInspector();

	void Update() override;

	Component* GetComponentToDelete() const;
	void SetComponentToDelete(Component* comp);

private:
	Component* componentToDelete = nullptr; // Setted in PanelInspector, when using the button "Delete" on the Component Options menu. If not nullptr, the PostUpdate() will delete the component it points to.

	void AddParticleComponentsOptions(GameObject* selected);
	void AddAudioComponentsOptions(GameObject* selected);
	void AddUIComponentsOptions(GameObject* selected);
	void AddColliderComponentsOptions(GameObject* selected);
	void AddNavigationComponentsOptions(GameObject* selected);
	
};
