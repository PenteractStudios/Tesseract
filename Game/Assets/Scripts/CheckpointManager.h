#pragma once

#include "Scripting/Script.h"

#define N_CHECKPOINTS 5

extern int checkpoint;

/// <summary>
/// This class should only have an instance on a particular scene, it will check the player's position every X seconds
/// If the player gets distanceThreshold close to one of the checkpoint positions, that one is set as checkpoint
/// Resetting the level through the canvas button that calls ReloadScene will reset player pos to last checkpoint
/// </summary>
class CheckpointManager : public Script {
	GENERATE_BODY(CheckpointManager);

public:

	void Start() override;
	void Update() override;
public:
	UID avatarUID;
	GameObject* avatarObj;
	float3 checkpointPosition1, checkpointPosition2, checkpointPosition3, checkpointPosition4, checkpointPosition5;
	float distanceThreshold, timeBetweenChecks, timeBetweenChecksCounter;

private:
	void CheckDistanceWithCheckpoints();

private:
	float3 runtimeCheckpointPositions[N_CHECKPOINTS];

};

