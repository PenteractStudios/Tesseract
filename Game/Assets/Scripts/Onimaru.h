#pragma once

#include "Player.h"

class OnimaruBullet;
class HUDController;
class Shield;

class Onimaru : public Player {
public:

	enum class ONIMARU_AUDIOS {
		SHIELD_ON,
		SHIELD_BLOCK,
		SHIELD_OFF,
		ENERGY_BLAST,
		ULTIMATE,
		SHOOT,
		SPECIAL_SHOOT,
		FOOTSTEP_RIGHT,
		FOOTSTEP_LEFT,
		HIT,
		DEATH,
		TOTAL
	};

	enum ONIMARU_STATES {
		IDLE,
		RUN_BACKWARD,
		RUN_FORWARD,
		RUN_LEFT,
		RUN_RIGHT,
		BLAST,
		NONE1,
		ULTI_INTRO,
		ULTI_LOOP,
		DEATH,
		SHOOTING,
		SHIELD,
		SHOOTSHIELD,
		RUNFORWARDLEFT,
		RUNFORWARDRIGHT,
		RUNBACKWARDLEFT,
		RUNBACKWARDRIGHT
	};

	std::vector<std::string> states{ "Idle" ,
					"RunBackward" , "RunForward" , "RunLeft" , "RunRight" ,
					"EnergyBlast", "" , "UltiIntro" , "UltiLoop" ,
					"Death" , "Shooting", "Shield","ShootingShield" ,
					"RunForwardLeft","RunForwardRight", "RunBackwardLeft", "RunBackwardRight"
	};

	//Onimaru ultimate related
	float originalAttackSpeed = 0.0f;
	float ultimateAttackSpeed = 20.0f;
	float ultimateTimeRemaining = 0.0f;
	float ultimateTotalTime = 3.0f;
	float ultimateRotationSpeed = 2.0f;
	float maxBulletSpread = 5.0f;
	// Blast ability
	float blastCooldown = 7.f;
	float blastDistance = 15.f;
	float blastAngle = 50.f;

public:
	// ------- Contructors ------- //
	Onimaru() {};
	void Init(UID onimaruUID = 0, UID onimaruBulletUID = 0, UID onimaruGunUID = 0, UID onimaruRightHand = 0, UID shieldUID = 0, UID onimaruTransformForUltimateProjectileOriginUID = 0, UID cameraUID = 0, UID canvasUID = 0, float maxSpread = 5.0f);
	void Update(bool lockMovement = false, bool lockRotation = false) override;
	void CheckCoolDowns(bool noCooldownMode = false) override;
	bool CanSwitch() const override;
	void OnAnimationFinished() override;
	void GetHit(float damage_) override;
	void OnDeath() override;
	void OnAnimationSecondaryFinished() override;
	void OnAnimationEvent(StateMachineEnum stateMachineEnum, const char* eventName);

	float GetRealUltimateCooldown();

	Quat GetSlightRandomSpread(float minValue, float maxValue) const;


	// Abilities' cooldowns
	float GetRealBlastCooldown();
	float GetRealShieldCooldown();

	/* Update enemies' vector */
	void AddEnemy(GameObject* enemy);
	void RemoveEnemy(GameObject* enemy);
	bool IsShielding() const;

private:

	ResourcePrefab* trail = nullptr;
	ResourcePrefab* bullet = nullptr;
	ResourcePrefab* ultimateBullet = nullptr;
	ComponentParticleSystem* ultimateSystem = nullptr;
	ComponentTransform* gunTransform = nullptr;

	ComponentParticleSystem* ultimateParticles = nullptr;
	ComponentTransform* transformForUltimateProjectileOrigin = nullptr;

	ComponentTransform* rightHand = nullptr;
	ComponentParticleSystem* blastParticles = nullptr;

	ComponentParticleSystem* shieldParticles = nullptr;

	Shield* shield = nullptr;
	GameObject* shieldGO = nullptr;

	bool shieldInCooldown = false;
	float shieldCooldownRemaining = 0.f;

	bool blastInUse = false;
	bool ultimateInUse = false;


	// Blast ability
	float blastDuration = 1.5f;
	float currentBlastDuration = 0.f;
	float blastCooldownRemaining = 0.f;
	float blastRemaining = 0.f;
	bool blastInCooldown = false;
	bool calculateEnemiesInRange = true;

	// HUD
	HUDController* hudControllerScript = nullptr;

	//Audio
	ComponentAudioSource* onimaruAudios[static_cast<int>(ONIMARU_AUDIOS::TOTAL)] = { nullptr };


	std::vector<GameObject*> enemiesInMap;
private:

	bool CanShoot() override;
	bool CanBlast();
	void Shoot() override;
	void Blast();
	void PlayAnimation();

	void StartUltimate();
	void FinishUltimate();
	void InitShield();
	void FadeShield();
	bool CanShield();
	bool CanUltimate();
};
