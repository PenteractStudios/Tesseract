#include "ParticleMotionState.h"

#include "Math/float3x3.h"

ParticleMotionState::ParticleMotionState(ComponentParticleSystem::Particle* p)
	: particle(p) {
}

ParticleMotionState::~ParticleMotionState() {
}

void ParticleMotionState::getWorldTransform(btTransform& centerOfMassWorldTrans) const {
	float4x4 particleModel = float4x4::FromTRS(particle->position, particle->rotation, float3::one);
	if (particle->emitter->GetIsAttachEmitter()) {
		float4x4 emitterModel;
		particle->emitter->ObtainEmitterGlobalMatrix(emitterModel);
		particleModel = emitterModel * particleModel;
	}
	float3 pos = particleModel.TranslatePart();
	Quat rot = particleModel.RotatePart().ToQuat();
	centerOfMassWorldTrans = btTransform(btQuaternion(rot.x, rot.y, rot.z, rot.w), btVector3(pos.x, pos.y, pos.z));
}

void ParticleMotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans) {
}
