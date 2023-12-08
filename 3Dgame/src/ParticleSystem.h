#pragma once

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void removeForces() { forces.clear(); }
	void remove(int);
	void update();
	void setLifespan(float);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
	ofVec3f gravity;
public:
	void set(const ofVec3f &g) { gravity = g; }
	GravityForce(const ofVec3f & gravity);
	GravityForce() {}
	void updateForce(Particle *);
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;
public:
	void set(const ofVec3f &min, const ofVec3f &max) { tmin = min; tmax = max; }
	TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
	TurbulenceForce() { tmin.set(0, 0, 0); tmax.set(0, 0, 0); }
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude = 1.0;
	float height = .2;
public:
	void set(float mag) { magnitude = mag; }
	void setHeight(float h) { height = h; }
	ImpulseRadialForce(float magnitude);
	ImpulseRadialForce() {}
	void updateForce(Particle *);
};

class CyclicForce : public ParticleForce {
	float magnitude = 1.0;
public:
	void set(float mag) { magnitude = mag; }
	CyclicForce(float magnitude);  
	CyclicForce() {}
	void updateForce(Particle *);
};

class ThrustForce : public ParticleForce {
	ofVec3f thrust;
public:
	void set(const ofVec3f &t) { thrust = t; }
	ThrustForce(const ofVec3f &t);
	ThrustForce() {}
	void updateForce(Particle *);
	void add(const ofVec3f &);
};

class ImpulseForce : public ParticleForce {
	ofVec3f impulse;
public:
	void set(const ofVec3f &i) { impulse = i; }
	ImpulseForce(const ofVec3f &impulse);
	ImpulseForce() {}
	void updateForce(Particle *);
	void add(const ofVec3f &);
};