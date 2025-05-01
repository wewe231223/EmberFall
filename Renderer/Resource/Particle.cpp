#include "pch.h"
#include "Particle.h"

Particle::Particle(EmitParticleContext* context) {
	mContext = context;
}

EmitParticleContext* Particle::Get() {
	return mContext;
}
