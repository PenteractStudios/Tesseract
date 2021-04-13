#include "ModuleUserInterface.h"

#include "GameObject.h"
#include "Components/UI/ComponentCanvas.h"
#include "Components/UI/ComponentSelectable.h"
#include "Components/UI/ComponentEventSystem.h"
#include "Components/UI/ComponentEventSystem.h"
#include "Components/ComponentBoundingBox2D.h"
#include "Application.h"
#include "ModuleFiles.h"
#include "ModuleEvents.h"
#include "Modules/ModuleResources.h"
#include "ModuleScene.h"
#include "UI/Interfaces/IPointerEnterHandler.h"
#include "UI/Interfaces/IPointerExitHandler.h"
#include "UI/Interfaces/IMouseClickHandler.h"
#include "Scene.h"
#include "Event.h"
#include "Resources/ResourceFont.h"
#include "Utils/FileDialog.h"
#include "FileSystem/TextureImporter.h"
#include "Utils/Logging.h"

#include "GL/glew.h"

#include "Utils/Leaks.h"

bool ModuleUserInterface::Init() {
	App->events->AddObserverToEvent(EventType::MOUSE_UPDATE, this);
	App->events->AddObserverToEvent(EventType::MOUSE_CLICKED, this);
	App->events->AddObserverToEvent(EventType::MOUSE_RELEASED, this);

	return true;
}

bool ModuleUserInterface::Start() {
	CreateQuadVBO();
	return true;
}

Character ModuleUserInterface::GetCharacter(UID font, char c) {
	ResourceFont* fontResource = (ResourceFont*) App->resources->GetResource(font);

	if (fontResource == nullptr) {
		return Character();
	}
	return fontResource->characters[c];
}

void ModuleUserInterface::GetCharactersInString(UID font, const std::string& sentence, std::vector<Character>& charsInSentence) {
	ResourceFont* fontResource = (ResourceFont*) App->resources->GetResource(font);

	if (fontResource == nullptr) {
		return;
	}

	for (std::string::const_iterator i = sentence.begin(); i != sentence.end(); ++i) {
		charsInSentence.push_back(fontResource->characters[*i]);
	}
}

void ModuleUserInterface::Render() {
	Scene* scene = App->scene->scene;
	if (scene != nullptr) {
		for (ComponentCanvasRenderer canvasRenderer : scene->canvasRendererComponents) {
			if (canvasRenderer.GetOwner().IsActiveInHierarchy()) {
				canvasRenderer.Render(&canvasRenderer.GetOwner());
			}
		}
	}
}

GameObject* ModuleUserInterface::GetCanvas() const {
	return canvas;
}

void ModuleUserInterface::ReceiveEvent(const Event& e) {
	float2 mousePos = float2(e.mouseUpdate.mouseX, e.mouseUpdate.mouseY);

	switch (e.type) {
	case EventType::MOUSE_UPDATE:
		if (currentEvSys) {
			for (ComponentSelectable& selectable : App->scene->scene->selectableComponents) {
				ComponentBoundingBox2D* bb = selectable.GetOwner().GetComponent<ComponentBoundingBox2D>();

				if (!selectable.IsHovered()) {
					if (bb->GetWorldAABB().Contains(mousePos)) {
						selectable.OnPointerEnter();
					}
				} else {
					if (!bb->GetWorldAABB().Contains(mousePos)) {
						selectable.OnPointerExit();
					}
				}
			}
		}
		break;

	case EventType::MOUSE_CLICKED:
		if (currentEvSys != nullptr) {
			ComponentSelectable* lastHoveredSelectable = currentEvSys->GetCurrentlyHovered();
			if (lastHoveredSelectable != nullptr) {
				if (lastHoveredSelectable->IsInteractable()) {
					IMouseClickHandler* mouseClickHandler = dynamic_cast<IMouseClickHandler*>(lastHoveredSelectable->GetSelectableComponent());

					if (mouseClickHandler != nullptr) {
						mouseClickHandler->OnClickedInternal();
						currentEvSys->SetClickedGameObject(&lastHoveredSelectable->GetOwner());
					}
				}
			} else {
				//Set selected to null
				currentEvSys->SetSelected(0);
			}
		}
		break;

	case EventType::MOUSE_RELEASED:
		if (currentEvSys != nullptr) {
			ComponentSelectable* lastHoveredSelectable = currentEvSys->GetCurrentlyHovered();
			if (lastHoveredSelectable != nullptr) {
				if (&lastHoveredSelectable->GetOwner() == currentEvSys->GetClickedGameObject()) {
					IMouseClickHandler* mouseClickHandler = dynamic_cast<IMouseClickHandler*>(lastHoveredSelectable->GetSelectableComponent());
					if (mouseClickHandler != nullptr) {
						mouseClickHandler->OnClicked();
						currentEvSys->SetClickedGameObject(&lastHoveredSelectable->GetOwner());
					}
				}
			}
		}

		break;
	default:
		break;
	}
}

bool ModuleUserInterface::CleanUp() {
	glDeleteBuffers(1, &quadVBO);
	return true;
}

unsigned int ModuleUserInterface::GetQuadVBO() {
	return quadVBO;
}

void ModuleUserInterface::CreateQuadVBO() {
	float buffer_data[] = {
		-0.5f,
		-0.5f,
		0.0f, //  v0 pos

		0.5f,
		-0.5f,
		0.0f, // v1 pos

		-0.5f,
		0.5f,
		0.0f, //  v2 pos

		0.5f,
		-0.5f,
		0.0f, //  v3 pos

		0.5f,
		0.5f,
		0.0f, // v4 pos

		-0.5f,
		0.5f,
		0.0f, //  v5 pos

		0.0f,
		0.0f, //  v0 texcoord

		1.0f,
		0.0f, //  v1 texcoord

		0.0f,
		1.0f, //  v2 texcoord

		1.0f,
		0.0f, //  v3 texcoord

		1.0f,
		1.0f, //  v4 texcoord

		0.0f,
		1.0f //  v5 texcoord
	};

	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // set vbo active
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);
}

void ModuleUserInterface::SetCurrentEventSystem(ComponentEventSystem* ev) {
	currentEvSys = ev;
}

ComponentEventSystem* ModuleUserInterface::GetCurrentEventSystem() {
	return currentEvSys;
}
