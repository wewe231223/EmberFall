#pragma once 
#include "../Utility/Defines.h"

class Particle {
public:
	Particle() = default;
	Particle(EmitParticleContext* context);

	~Particle() = default;

	Particle(const Particle&) = default;
	Particle& operator=(const Particle&) = default;

	Particle(Particle&&) = default;
	Particle& operator=(Particle&&) = default;
public:
	EmitParticleContext* Get(); 
private:
	EmitParticleContext* mContext{}; 
};