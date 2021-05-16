#pragma once

#include "Components/Component.h"

#include <vector>

class GameObject;
class ComponentSelectable;
class IPointerEnterHandler;

class ComponentEventSystem : public Component {
public:
	REGISTER_COMPONENT(ComponentEventSystem, ComponentType::EVENT_SYSTEM, false);
	~ComponentEventSystem();

	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	void OnEnable() override;
	void OnDisable() override;

	void SetSelected(UID newSelectableComponentId);
	void EnteredPointerOnSelectable(ComponentSelectable* newHoveredComponent);	//Interface implementation
	void ExitedPointerOnSelectable(ComponentSelectable* newUnHoveredComponent); //Interface implementation
	ComponentSelectable* GetCurrentSelected() const;							//Returns currently selected ComponentSelectable
	ComponentSelectable* GetCurrentlyHovered() const;							//Returns last Selectable that was hovered over with mouse

private:
	UID selectedId = 0; //Currently selected SelectableComponent*

	std::vector<UID> hoveredSelectableIds; //vector of SelectableComponents* it updates (adding/removing) with mouse events

public:
	UID firstSelectedId = 0; //Reference to the "first selected selectableComponent", this is not used directly but Unity implements it so that users can access it
};
