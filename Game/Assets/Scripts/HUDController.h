#pragma once

#include "Scripting/Script.h"

class GameObject;
class ComponentText;

static enum Cooldowns {
	FANG_SKILL_1,
	FANG_SKILL_2,
	FANG_SKILL_3,
	ONIMARU_SKILL_1,
	ONIMARU_SKILL_2,
	ONIMARU_SKILL_3,
	SWITCH_SKILL,
	TOTAL
};

class HUDController : public Script
{
	GENERATE_BODY(HUDController);

public:

	void Start() override;
	void Update() override;

public:
	UID fangUID = 0;
	UID onimaruUID = 0;

	UID fangMainCanvasUID = 0;
	UID onimaruMainCanvasUID = 0;

	UID fangSkillsMainCanvasUID = 0;
	UID onimaruSkillsMainCanvasUID = 0;

	UID fangSkillsSecondCanvasUID = 0;
	UID onimaruSkillsSecondCanvasUID = 0;

	UID fangHealthMainCanvasUID = 0;
	UID onimaruHealthMainCanvasUID = 0;

	UID fangHealthSecondCanvasUID = 0;
	UID onimaruHealthSecondCanvasUID = 0;

	UID swapingSkillCanvasUID = 0;

	UID scoreTextUID = 0;

	static void UpdateCooldowns(float onimaruCooldown1, float onimaruCooldown2, float onimaruCooldown3,
		float fangCooldown1, float fangCooldown2, float fangCooldown3,
		float switchCooldown);

	static void UpdateHP(float currentHp, float altHp);

	static void ChangePlayerHUD();

	void UpdateScore(int score_);

private:
	static void UpdateComponents();
	static void UpdateCommonSkill();
	static void UpdateFangCooldowns(GameObject* fangSkillCanvas);
	static void UpdateOnimaruCooldowns(GameObject* onimaruSkillCanvas);
	static void UpdateCanvasHP(GameObject* targetCanvas, int health, bool darkened);

private:
	static GameObject* fang;
	static GameObject* onimaru;

	static GameObject* fangCanvas;
	static GameObject* onimaruCanvas;

	static GameObject* fangSkillsMainCanvas;
	static GameObject* onimaruSkillsMainCanvas;
	static GameObject* fangSkillsSecondCanvas;
	static GameObject* onimaruSkillsSecondCanvas;

	static GameObject* fangHealthMainCanvas;
	static GameObject* onimaruHealthMainCanvas;
	static GameObject* fangHealthSecondCanvas;
	static GameObject* onimaruHealthSecondCanvas;

	static GameObject* swapingSkillCanvas;

	static std::array<float, Cooldowns::TOTAL> cooldowns;

	/* COLORS */

	static const float4 colorMagenta;
	static const float4 colorWhite;
	
	static const float4 colorMagentaDarkened;
	static const float4 colorWhiteDarkened;

	ComponentText* scoreText = nullptr;

	int score = 0;
};

