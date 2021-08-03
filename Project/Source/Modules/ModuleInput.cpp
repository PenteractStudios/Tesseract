#include "ModuleInput.h"

#include "Globals.h"
#include "Application.h"
#include "Modules/ModuleWindow.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleEvents.h"
#include "Panels/PanelScene.h"
#include "Utils/Logging.h"

#include "imgui_impl_sdl.h"
#include "SDL.h"
#include "Brofiler.h"

#include "Utils/Leaks.h"

bool ModuleInput::Init() {
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0) {
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

UpdateStatus ModuleInput::PreUpdate() {
	BROFILER_CATEGORY("ModuleInput - PreUpdate", Profiler::Color::AntiqueWhite)

	ImGuiIO& io = ImGui::GetIO();

	mouseMotion = {0, 0};
	mouseWheelMotion = 0;

	int windowId = SDL_GetWindowID(App->window->window);

	for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
		if (keyboard[i] == KeyState::KS_DOWN) {
			keyboard[i] = KeyState::KS_REPEAT;
		}

		if (keyboard[i] == KeyState::KS_UP) {
			keyboard[i] = KeyState::KS_IDLE;
		}
	}

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
		if (mouseButtons[i] == KeyState::KS_DOWN) {
			mouseButtons[i] = KeyState::KS_REPEAT;
		}

		if (mouseButtons[i] == KeyState::KS_UP) {
			mouseButtons[i] = KeyState::KS_IDLE;
		}
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (playerControllers[i] == nullptr) continue;
		for (int j = 0; j < SDL_CONTROLLER_BUTTON_MAX; ++j) {
			if (playerControllers[i]->gameControllerButtons[j] == KeyState::KS_DOWN) {
				playerControllers[i]->gameControllerButtons[j] = KeyState::KS_REPEAT;
			}

			if (playerControllers[i]->gameControllerButtons[j] == KeyState::KS_UP) {
				playerControllers[i]->gameControllerButtons[j] = KeyState::KS_IDLE;
			}
		}
	}

	int auxMouseX;
	int auxMouseY;
	SDL_GetGlobalMouseState(&auxMouseX, &auxMouseY);

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
		case SDL_QUIT:
			return UpdateStatus::STOP;
		case SDL_CONTROLLERDEVICEADDED:
			OnControllerAdded(event.cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			LOG("Controllerd %d was remapped", event.cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			OnControllerRemoved(event.cdevice.which);
			break;
		case SDL_CONTROLLERAXISMOTION: {
			PlayerController* player = GetPlayerControllerWithJoystickIndex(event.cdevice.which);
			if (!player) break;

			if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
				player->gameControllerAxises[event.caxis.axis] = event.caxis.value;

				break;
			}

			//Left of dead zone
			if (event.caxis.value < -JOYSTICK_DEAD_ZONE) {
				player->gameControllerAxises[event.caxis.axis] = event.caxis.value;
			}
			//Right of dead zone
			else if (event.caxis.value > JOYSTICK_DEAD_ZONE) {
				player->gameControllerAxises[event.caxis.axis] = event.caxis.value;

			} else {
				player->gameControllerAxises[event.caxis.axis] = 0;
			}

			break;
		}
		case SDL_CONTROLLERBUTTONDOWN: {
			if (GetPlayerControllerWithJoystickIndex(event.cdevice.which)) {
				GetPlayerControllerWithJoystickIndex(event.cdevice.which)->gameControllerButtons[event.cbutton.button] = KeyState::KS_DOWN;
			}
			break;
		}
		case SDL_CONTROLLERBUTTONUP:
			if (GetPlayerControllerWithJoystickIndex(event.cdevice.which)) {
				GetPlayerControllerWithJoystickIndex(event.cdevice.which)->gameControllerButtons[event.cbutton.button] = KeyState::KS_UP;
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.windowID == windowId) {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					return UpdateStatus::STOP;
				case SDL_WINDOWEVENT_RESIZED:

					TesseractEvent resizeEvent(TesseractEventType::SCREEN_RESIZED);
					resizeEvent.Set<ViewportResizedStruct>(event.window.data1, event.window.data2);
					App->events->AddEvent(resizeEvent);

					break;
				}
			}
			break;

		case SDL_DROPFILE:
			ReleaseDroppedFilePath();
			droppedFilePath = event.drop.file;
			LOG("File dropped: %s", droppedFilePath);
			break;

		case SDL_MOUSEWHEEL:
			if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
				mouseWheelMotion = (float) event.wheel.x;
			} else {
				mouseWheelMotion = (float) event.wheel.y;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouseButtons[event.button.button - 1] = KeyState::KS_DOWN;
			if (event.button.button == SDL_BUTTON_LEFT) {
#if !GAME
				App->editor->OnMouseClicked();
#else
				TesseractEvent e(TesseractEventType::MOUSE_CLICKED);
				App->events->AddEvent(e);
#endif
			}
			break;
		case SDL_MOUSEBUTTONUP:
			mouseButtons[event.button.button - 1] = KeyState::KS_UP;
			if (event.button.button == SDL_BUTTON_LEFT) {
#if !GAME
				App->editor->OnMouseReleased();
#else
				App->events->AddEvent(TesseractEventType::MOUSE_RELEASED);
#endif
			}
			break;

		case SDL_KEYDOWN:
			keyboard[event.key.keysym.scancode] = KeyState::KS_DOWN;
			break;

		case SDL_KEYUP:
			keyboard[event.key.keysym.scancode] = KeyState::KS_UP;
			break;
		}
	}

	if (io.WantCaptureKeyboard) {
		for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
			keyboard[i] = KeyState::KS_IDLE;
		}
	}

	if (io.WantCaptureMouse) {
		mouseWheelMotion = 0;
		mouseMotion.x = 0;
		mouseMotion.y = 0;

		for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i) {
			mouseButtons[i] = KeyState::KS_IDLE;
		}
	} else {
		int mouseX;
		int mouseY;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);
		if (!mouseWarped) {
			mouseMotion.x = mouseX - mouse.x;
			mouseMotion.y = mouseY - mouse.y;
		} else {
			mouseWarped = false;
		}
		mouse.x = (float) mouseX;
		mouse.y = (float) mouseY;
	}

	return UpdateStatus::CONTINUE;
}

bool ModuleInput::CleanUp() {
	ReleaseDroppedFilePath();
	LOG("Quitting SDL input event subsystem");

	for (int i = 0; i < MAX_PLAYERS; i++) {
		RELEASE(playerControllers[i]);
	}

	SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
	return true;
}

void ModuleInput::ReleaseDroppedFilePath() {
	if (droppedFilePath != nullptr) {
		SDL_free(droppedFilePath);
		droppedFilePath = nullptr;
	}
}

void ModuleInput::WarpMouse(int mouseX, int mouseY) {
	SDL_WarpMouseGlobal(mouseX, mouseY);
	mouseWarped = true;
}

const char* ModuleInput::GetDroppedFilePath() const {
	return droppedFilePath;
}

KeyState ModuleInput::GetKey(int scancode) const {
	return keyboard[scancode];
}

KeyState ModuleInput::GetMouseButton(int button) const {
	return mouseButtons[button - 1];
}

float ModuleInput::GetMouseWheelMotion() const {
	return mouseWheelMotion;
}

const float2& ModuleInput::GetMouseMotion() const {
	return mouseMotion;
}

const float2& ModuleInput::GetMousePosition(bool sceneSensitive) const {
	if (!sceneSensitive) {
		return mouse;
	}
#if !GAME
	return App->editor->panelScene.GetMousePosOnScene();
#else
	return mouse;
#endif
}

KeyState* ModuleInput::GetMouseButtons() {
	return mouseButtons;
}

KeyState* ModuleInput::GetKeyboard() {
	return keyboard;
}

void ModuleInput::OnControllerAdded(int index) {
	LOG("Added controller");
	if (SDL_IsGameController(index)) {
		LOG("New controller was deemed compatible, type is %s", GetControllerTypeAsString(SDL_GameControllerTypeForIndex(index)));

		if (playerControllers[0] == nullptr) {
			LOG("New controller took player slot 0");
			playerControllers[0] = new PlayerController(index);
			playerControllers[0]->StartSimpleControllerVibration(0.5f, 400);
		} else if (playerControllers[1] == nullptr) {
			LOG("New controller took player slot 1");
			playerControllers[1] = new PlayerController(index);
			playerControllers[1]->StartSimpleControllerVibration(0.5f, 400);
		} else {
			LOG("No available slots found, Game Controller will be ignored");
		}
	}
}

void ModuleInput::OnControllerRemoved(int index) {
	LOG("Disconnected controller");
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (playerControllers[i] == nullptr) continue;
		if (playerControllers[i]->joystickIndex == index) {
			RELEASE(playerControllers[i]);
			LOG("Player slot %d is now empty", i);
		}
	}
}

PlayerController* ModuleInput::GetPlayerControllerWithJoystickIndex(int joystickIndex) const {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (playerControllers[i] == nullptr) continue;
		if (playerControllers[i]->joystickIndex == joystickIndex) {
			return playerControllers[i];
		}
	}
	return nullptr;
}

PlayerController* ModuleInput::GetPlayerController(int index) const {
	return playerControllers[index];
}

const char* ModuleInput::GetControllerTypeAsString(SDL_GameControllerType type) {
	switch (type) {
	case SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS3:
		return "PS3 Controller";
	case SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS4:
		return "PS4 Controller";
	case SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOX360:
		return "XBOX 360 Controller";
	case SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOXONE:
		return "XBOX One Controller";
	case SDL_GameControllerType::SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
		return "Nintendo Switch Pro Controller";
	default:
		return "UNKNOWN";
	}
}
