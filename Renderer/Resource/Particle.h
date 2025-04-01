#pragma once 
#include "../Utility/Defines.h"

class Particle {
public:
	Particle();
	~Particle();

private:
	EmitParticleContext* context{}; 

};