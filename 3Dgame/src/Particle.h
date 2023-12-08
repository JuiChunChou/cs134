#pragma once

#include "ofMain.h"

class ParticleForceField;

class Particle {
public:
	Particle();

	ofVec3f position;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float   rForce = 0;
	float	rVelocity = 0;
	float	rAcceleration = 0;
	float   rotation;
	float	damping;
	float   mass;
	float   lifespan;
	float   radius;
	float   birthtime;
	void    integrate();
	void    draw();
	float   age();        // sec
	ofColor color;
};


