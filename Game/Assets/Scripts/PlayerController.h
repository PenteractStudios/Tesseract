#pragma once

#include "Player.h"
#include "Scripting/Script.h"
#include "Math/float3.h"
#include <vector>

class GameObject;
class ComponentTransform;
class ComponentCamera;
class ComponentAudioSource;
class ComponentParticleSystem;
class HUDController;
class OnimaruBullet;
class ComponentAgent;
class ComponentAnimation;
class State;
class ResourcePrefab;
struct TesseractEvent;

// We should get these two values from the Character class

#define ONIMARU_MAX_HEALTH 10.0f 
#define FANG_MAX_HEALTH 10.0f

enum class MovementDirection {
	NONE = 0, UP = 1, UP_LEFT = 2, LEFT = 3, DOWN_LEFT = 4, DOWN = 5, DOWN_RIGHT = 6, RIGHT = 7, UP_RIGHT = 8
};

class PlayerController : public Script {
	GENERATE_BODY(PlayerController);

public:
	enum class AudioType {
		DASH,
		SWITCH,
		SHOOT,
		FANGHIT,
		ONIHIT,
		FANGDEATH,
		ONIDEATH,
		TOTAL
	};

	void Start() override;
	void Update() override;

	bool IsDead();
	void SetInvincible(bool status);
	void SetOverpower(bool status);
	void SetNoCooldown(bool status);
	void TakeDamage(bool ranged = true);
	int GetOverPowerMode();
public:
	bool debugGetHit = false;
	GameObject* player = nullptr;
	GameObject* camera = nullptr;
	GameObject* fang = nullptr;
	GameObject* onimaru = nullptr;

	ComponentAgent* agent = nullptr;

	ComponentAnimation* fangAnimation = nullptr;


	UID fangUID = 0;
	UID fangTrailUID = 0;
	UID fangLeftGunUID = 0;
	UID fangRightGunUID = 0;
	UID fangBulletUID = 0;

	UID onimaruUID = 0;
	UID onimaruParticleUID = 0;
	UID onimaruBulletUID = 0;
	UID onimaruGunUID = 0;

	UID switchParticlesUID = 0;

	UID mainNodeUID = 0;
	UID cameraUID = 0;
	UID canvasUID = 0;

	bool rightShot = true;
	bool shooting = false;

	bool hitTaken = false;

	Player fangCharacter = Player(10, 10.0f, 3, 1.0f);
	Player onimaruCharacter = Player(10, 6.0f, 1, 1.0f);

	float distanceRayCast = 2.f;
	float switchCooldown = 5.f;

	float dashCooldown = 5.f;
	float dashSpeed = 100.f;
	float dashDuration = 0.1f;

	float cameraOffsetZ = 20.f;
	float cameraOffsetY = 10.f;
	float cameraOffsetX = 0.f;

	int fangRecoveryRate = 1.0f;
	int onimaruRecoveryRate = 1.0f;

	bool firstTime = true;

	bool switchInProgress = false;
	float switchDelay = 0.37f;

	/* Fang & onimaru damage */
	
	int rangedDamageTaken = 1;
	int meleeDamageTaken = 1;
	
	//Camera
	bool useSmoothCamera = true;
	float smoothCameraSpeed = 5.0f;

	std::vector<std::string> states{ "Idle" ,
								"RunBackward" , "RunForward" , "RunLeft" , "RunRight" ,
								"DashBackward", "DashForward" , "DashLeft" , "DashRight" ,
								"Death" , "Hurt" , "LeftShot" , "RightShot", "Shooting", "RunForwardLeft",
								"RunForwardRight", "RunBackwardLeft", "RunBarckwardRight"
	};

private:

	void PlayAnimation(MovementDirection md);
	void MoveTo(MovementDirection md);
	void InitDash(MovementDirection md);
	void Dash();
	void LookAtMouse();
	void CheckCoolDowns();
	void SwitchCharacter();
	void Shoot();
	void UpdatePlayerStats();
	void UpdateCameraPosition();
	bool CanDash();
	bool CanSwitch();
	bool CanShoot();
	void ResetSwitchStatus();

	float3 GetDirection(MovementDirection md) const;
	MovementDirection GetInputMovementDirection() const;
	int GetMouseDirectionState(MovementDirection input);
private:

	float dashCooldownRemaining = 0.f;
	bool dashInCooldown = false;
	bool dashing = false;
	float dashRemaining = 0.f;

	float switchCooldownRemaining = 0.f;
	bool switchInCooldown = false;

	float fangAttackCooldownRemaining = 0.f;
	float onimaruAttackCooldownRemaining = 0.f;
	bool invincibleMode = false;
	int overpowerMode = 1;
	bool noCooldownMode = false;

	float fangRecovering = 0.f;
	float onimaruRecovering = 0.f;
	float currentSwitchDelay = 0.f;
	bool playSwitchParticles = true;

	float3 initialPosition = float3(0, 0, 0);
	float3 dashDestination = float3(0, 0, 0);
	float3 dashDirection = float3(0, 0, 0);
	float3 facePointDir = float3(0, 0, 0);
	MovementDirection dashMovementDirection = MovementDirection::NONE;
	ComponentTransform* transform = nullptr;
	ComponentTransform* onimaruGunTransform = nullptr;
	ComponentCamera* compCamera = nullptr;
	ComponentTransform* cameraTransform = nullptr;

	GameObject* fangLeftGun = nullptr;
	GameObject* fangRightGun = nullptr;
	GameObject* onimaruGun = nullptr;
	ComponentTransform* fangRightGunTransform = nullptr;
	ComponentTransform* fangLeftGunTransform = nullptr;


	//Animation
	ComponentAnimation* onimaruAnimation = nullptr;
	State* fangCurrentState = nullptr;
	State* onimaruCurrentState = nullptr;

	ResourcePrefab* onimaruTrail = nullptr;
	ResourcePrefab* fangTrail = nullptr;
	ResourcePrefab* fangBullet = nullptr;
	ResourcePrefab* onimaruBullet = nullptr;

	GameObject* switchEffects = nullptr;

	//Particles
	ComponentParticleSystem* fangCompParticle = nullptr;
	ComponentParticleSystem* onimaruCompParticle = nullptr;

	//Audio
	ComponentAudioSource* audios[static_cast<int>(AudioType::TOTAL)] = { nullptr };

	HUDController* hudControllerScript = nullptr;
	OnimaruBullet* onimaruBulletcript = nullptr;
};
