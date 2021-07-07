#include "GameController.h"

#include "GameObject.h"
#include "Modules/ModuleCamera.h"
#include "GameplaySystems.h"

#include "Math/float3x3.h"
#include "Geometry/frustum.h"

EXPOSE_MEMBERS(GameController) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	MEMBER(MemberType::GAME_OBJECT_UID, gameCameraUID),
	MEMBER(MemberType::GAME_OBJECT_UID, godCameraUID),
	MEMBER(MemberType::GAME_OBJECT_UID, staticCamera1UID),
	MEMBER(MemberType::GAME_OBJECT_UID, staticCamera2UID),
	MEMBER(MemberType::GAME_OBJECT_UID, staticCamera3UID),
	MEMBER(MemberType::GAME_OBJECT_UID, staticCamera4UID),
	MEMBER(MemberType::GAME_OBJECT_UID, playerUID),
	MEMBER(MemberType::GAME_OBJECT_UID, pauseUID),
	MEMBER(MemberType::GAME_OBJECT_UID, hudUID),
	MEMBER(MemberType::GAME_OBJECT_UID, godModeControllerUID),
	MEMBER(MemberType::FLOAT, speed),
	MEMBER(MemberType::FLOAT, rotationSpeedX),
	MEMBER(MemberType::FLOAT, rotationSpeedY),
	MEMBER(MemberType::FLOAT, focusDistance),
	MEMBER(MemberType::FLOAT, transitionSpeed),
};

GENERATE_BODY_IMPL(GameController);

void GameController::Start() {

	showWireframe = false;
	transitionFinished = false;
	Debug::Log("SCRIPT STARTED");

	gameCamera = GameplaySystems::GetGameObject(gameCameraUID);
	godCamera = GameplaySystems::GetGameObject(godCameraUID);
	staticCamera1 = GameplaySystems::GetGameObject(staticCamera1UID) ? GameplaySystems::GetGameObject(staticCamera1UID)->GetComponent<ComponentCamera>() : nullptr;
	staticCamera2 = GameplaySystems::GetGameObject(staticCamera2UID) ? GameplaySystems::GetGameObject(staticCamera2UID)->GetComponent<ComponentCamera>() : nullptr;
	staticCamera3 = GameplaySystems::GetGameObject(staticCamera3UID) ? GameplaySystems::GetGameObject(staticCamera3UID)->GetComponent<ComponentCamera>() : nullptr;
	staticCamera4 = GameplaySystems::GetGameObject(staticCamera4UID) ? GameplaySystems::GetGameObject(staticCamera4UID)->GetComponent<ComponentCamera>() : nullptr;

	player = GameplaySystems::GetGameObject(playerUID);

	pauseCanvas = GameplaySystems::GetGameObject(pauseUID);
	hudCanvas = GameplaySystems::GetGameObject(hudUID);

	if (gameCamera) {
		camera = gameCamera->GetComponent<ComponentCamera>();
		GameplaySystems::SetRenderCamera(camera);
	}

	Debug::SetGodModeOn(false);
	if (gameCamera && godCamera) godModeAvailable = true;
	godModeController = GameplaySystems::GetGameObject(godModeControllerUID);
}

void GameController::Update() {
	if (godModeController) {
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_G) && !isPaused) {
			if (godModeAvailable) {
				Debug::ToggleDebugMode();
				if (Debug::IsGodModeOn()) {
					Debug::SetGodModeOn(false);
					if (showWireframe) { // If Wireframe enabled when leaving God Mode, update to Shaded
						Debug::UpdateShadingMode("Shaded");
					}
					godModeController->Disable();
				} else {
					if (showWireframe) { // If Wireframe enabled when entering GodMode, update to Wireframe
						Debug::UpdateShadingMode("Wireframe");
					}
					Debug::SetGodModeOn(true);
					godModeController->Enable();
				}
			}
		}
	}
	if (pauseCanvas) {
		if (pauseCanvas->IsActive()) {
			isPaused = true;
		} else {
			isPaused = false;
		}
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_ESCAPE)) {
		if (pauseCanvas) {
			if (!isPaused) {
				Time::PauseGame();
				if (hudCanvas) hudCanvas->Disable();
				pauseCanvas->Enable();
			} else {
				Time::ResumeGame();
				if (hudCanvas) hudCanvas->Enable();
				pauseCanvas->Disable();
			}
		}
	}

	// Static cameras
	if (!Debug::IsGodModeOn() && !isPaused) {
		if (Input::GetKeyCode(Input::KEYCODE::KEY_0) && gameCamera) {
			camera = gameCamera->GetComponent<ComponentCamera>();
			GameplaySystems::SetRenderCamera(camera);
			Debug::SetGodModeOn(false);
		}
		if (Input::GetKeyCode(Input::KEYCODE::KEY_1) && staticCamera1) {
			GameplaySystems::SetRenderCamera(staticCamera1);
			Debug::SetGodModeOn(false);
		}
		if (Input::GetKeyCode(Input::KEYCODE::KEY_2) && staticCamera2) {
			GameplaySystems::SetRenderCamera(staticCamera2);
			Debug::SetGodModeOn(false);
		}
		if (Input::GetKeyCode(Input::KEYCODE::KEY_3) && staticCamera3) {
			GameplaySystems::SetRenderCamera(staticCamera3);
			Debug::SetGodModeOn(false);
		}
		if (Input::GetKeyCode(Input::KEYCODE::KEY_4) && staticCamera4) {
			GameplaySystems::SetRenderCamera(staticCamera4);
			Debug::SetGodModeOn(false);
		}
	}

	// Godmode Controls
	ComponentCamera* cameraGodMode = nullptr;
	ComponentTransform* transform = nullptr;
	if (godCamera) {
		transform = godCamera->GetComponent<ComponentTransform>();
		cameraGodMode = godCamera->GetComponent<ComponentCamera>();
	}
	if (!transform) return;
	if (!cameraGodMode) return;

	if (Debug::IsGodModeOn()) {
		// Movement
		// --- Forward
		if (Input::GetKeyCode(Input::KEYCODE::KEY_UP)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->Front().Normalized() * speed * Time::GetDeltaTime());
		}
		// --- Left
		if (Input::GetKeyCode(Input::KEYCODE::KEY_LEFT)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->WorldRight().Normalized() * -speed * Time::GetDeltaTime());
		}
		// --- Backward
		if (Input::GetKeyCode(Input::KEYCODE::KEY_DOWN)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->Front().Normalized() * -speed * Time::GetDeltaTime());
		}
		// --- Right
		if (Input::GetKeyCode(Input::KEYCODE::KEY_RIGHT)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->WorldRight().Normalized() * speed * Time::GetDeltaTime());
		}
		// --- Down
		if (Input::GetKeyCode(Input::KEYCODE::KEY_COMMA)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->Up().Normalized() * -speed * Time::GetDeltaTime());
		}
		if (Input::GetKeyCode(Input::KEYCODE::KEY_PERIOD)) {
			transform->SetPosition(transform->GetPosition() + cameraGodMode->GetFrustum()->Up().Normalized() * speed * Time::GetDeltaTime());
		}
		// Rotation
		if (Input::GetMouseButton(2)) { // TODO: Why a 2?! It should be a 3!
			if (Input::GetKeyCode(Input::KEYCODE::KEY_LALT)) {
				// --- Orbiting
				vec oldFocus = transform->GetPosition() + transform->GetLocalMatrix().Col3(2) * focusDistance;
				Rotate(Input::GetMouseMotion(), cameraGodMode->GetFrustum(), transform);
				vec newFocus = transform->GetPosition() + transform->GetLocalMatrix().Col3(2) * focusDistance;
				transform->SetPosition(transform->GetPosition() + (oldFocus - newFocus));
			} else {
				// --- Panning
				Rotate(Input::GetMouseMotion(), cameraGodMode->GetFrustum(), transform);
			}
		}

		// Func
		// --- Show/Hide DebugDraw
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_Z)) {
			Debug::ToggleDebugDraw();
		}
		// --- Show/Hide Wireframe
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_X)) {
			if (showWireframe) {
				Debug::UpdateShadingMode("Shaded");
			} else {
				Debug::UpdateShadingMode("Wireframe");
			}
			showWireframe = !showWireframe;
		}
		// --- Show/Hide Quadtree
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_C)) {
			Debug::ToggleDrawQuadtree();
		}
		// --- Show/Hide Bounding Boxes
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_V)) {
			Debug::ToggleDrawBBoxes();
		}
		// --- Show/Hide Animation Bones
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_B)) {
			Debug::ToggleDrawAnimationBones();
		}
		// --- Show/Hide All Light Gizmos
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_N)) {
			Debug::ToggleDrawLightGizmos();
		}
		// --- Show/Hide All Camera Frustums
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_M)) {
			Debug::ToggleDrawCameraFrustums();
		}
		// --- Show/Hide Skybox
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_K)) {
			ComponentSkyBox* skybox = gameCamera->GetComponent<ComponentSkyBox>();
			if (skybox->IsActive()) {
				skybox->Disable();
			} else {
				skybox->Enable();
			}
		}
	}
}

void GameController::ReceiveEvent(TesseractEvent& e) {
	switch (e.type) {
	case TesseractEventType::PRESSED_STOP:
		if (showWireframe) Debug::UpdateShadingMode("Shaded");
		break;
	default:
		break;
	}
}

void GameController::Rotate(float2 mouseMotion, Frustum* frustum, ComponentTransform* transform) {
	Quat yIncrement = Quat::RotateY(-mouseMotion.x * rotationSpeedY * DEGTORAD * Time::GetDeltaTime());
	Quat xIncrement = Quat::RotateAxisAngle(frustum->WorldRight().Normalized(), -mouseMotion.y * rotationSpeedX * DEGTORAD * Time::GetDeltaTime());
	transform->SetRotation(yIncrement * xIncrement * transform->GetRotation());
}

void GameController::DoTransition() {
	if (player != nullptr) {
		float3 finalPosition = float3(-164, 478, 449);
		float3 currentPosition = gameCamera->GetComponent<ComponentTransform>()->GetPosition();

		if (currentPosition.x > finalPosition.x) {
			currentPosition.x -= transitionSpeed * Time::GetDeltaTime();
			gameCamera->GetComponent<ComponentTransform>()->SetPosition(currentPosition);
		} else {
			transitionFinished = true;
			gameCamera->GetComponent<ComponentTransform>()->SetPosition(finalPosition);
		}
	}
}
