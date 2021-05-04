#pragma once

#include "Components/Component.h"
#include "LinearMath/btMotionState.h"
#include "LinearMath/btTransform.h"

#include "Math/float3.h"

class MotionState : public btMotionState {
public:
	MotionState(Component* componentCollider, float3 centerOffset, bool freezeRot);
	~MotionState();

	void getWorldTransform(btTransform& centerOfMassWorldTrans) const;
	void setWorldTransform(const btTransform& centerOfMassWorldTrans);

public:
	bool freezeRotation = false;

private:
	Component* collider = nullptr;
	btTransform massCenterOffset = btTransform::getIdentity();
};
