#pragma once

#include "Player.h"
#include "Scripting/Script.h"
#include "Math/float3.h"
#include <vector>
#include "Fang.h"
#include "Onimaru.h"

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



class PlayerController : public Script {
	GENERATE_BODY(PlayerController);

public:
	enum class AudioType {
		SWITCH,
		TOTAL
	};

	void Start() override;
	void Update() override;

	//Debug
	void SetInvincible(bool status);
	void SetOverpower(bool status);
	void SetNoCooldown(bool status);
	int GetOverPowerMode();
	bool IsPlayerDead() { return !playerFang.isAlive || !playerOnimaru.isAlive; }
	void TakeDamage(bool ranged = false);

	void AddEnemyInMap(GameObject* enemy);
	void RemoveEnemyFromMap(GameObject* enemy);
public:
	//Debug
	bool invincibleMode = false;
	int overpowerMode = 1;
	bool noCooldownMode = false;
	bool debugGetHit = false;

	Onimaru playerOnimaru = Onimaru();
	Fang playerFang = Fang();

	//MainGameObject
	UID mainNodeUID = 0;

	//Fang
	UID fangUID = 0;
	UID fangTrailUID = 0;
	UID fangLeftGunUID = 0;
	UID fangRightGunUID = 0;
	UID fangBulletUID = 0;
	UID EMPUID = 0;
	UID fangUltimateUID = 0;

	//Onimaru
	UID onimaruUID = 0;
	UID onimaruParticleUID = 0;
	UID onimaruBulletUID = 0;
	UID onimaruGunUID = 0;
	UID onimaruUltimateProjectileOriginUID = 0;
	UID onimaruRightHandUID = 0;
	UID onimaruShieldUID = 0;

	//HUD
	UID canvasUID = 0;
	float fangRecoveryRate = 1.0f;
	float onimaruRecoveryRate = 1.0f;
	bool firstTime = true;
	bool hitTaken = false;

	//Camera
	UID cameraUID = 0;
	GameObject* camera = nullptr;
	float cameraOffsetZ = 20.f;
	float cameraOffsetY = 10.f;
	float cameraOffsetX = 0.f;
	bool useSmoothCamera = true;
	float smoothCameraSpeed = 5.0f;
	//float maxOnimaruBulletSpread = 5.0f;

	//Switch
	UID switchParticlesUID = 0;
	float switchCooldown = 5.f;
	bool switchInProgress = false;
	float switchDelay = 0.37f;

	/* Fang & onimaru damage */
	//split
	float rangedDamageTaken = 1.0f;
	float meleeDamageTaken = 1.0f;


private:
	void CheckCoolDowns();
	void SwitchCharacter();
	void UpdatePlayerStats();
	bool CanSwitch();
	void ResetSwitchStatus();
private:

	//Switch
	float switchCooldownRemaining = 0.f;
	bool switchInCooldown = false;

	//HUD
	float fangRecovering = 0.f;
	float onimaruRecovering = 0.f;
	HUDController* hudControllerScript = nullptr;

	//Switch
	float currentSwitchDelay = 0.f;
	bool playSwitchParticles = true;
	GameObject* switchEffects = nullptr;

	//Camera
	ComponentCamera* compCamera = nullptr;
	ComponentTransform* cameraTransform = nullptr;

	//Audio
	ComponentAudioSource* audios[static_cast<int>(AudioType::TOTAL)] = { nullptr };
};
