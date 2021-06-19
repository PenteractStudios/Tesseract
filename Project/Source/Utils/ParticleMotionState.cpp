#include "ParticleMotionState.h"

ParticleMotionState::ParticleMotionState(ComponentParticleSystem::Particle* p)
	: particle(p) {
}

ParticleMotionState::~ParticleMotionState() {
}

void ParticleMotionState::getWorldTransform(btTransform& centerOfMassWorldTrans) const {
	centerOfMassWorldTrans = btTransform(btQuaternion(particle->rotation.x, particle->rotation.y, particle->rotation.z), btVector3(particle->position.x, particle->position.y, particle->position.z));
}

void ParticleMotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans) {
}
