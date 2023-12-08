#pragma once
#include "ofMain.h"

class TransformObject {
protected:
	TransformObject();
	ofVec3f position, scale;
	float	rotation;
	bool	bSelected;
public:
	void setPosition(const ofVec3f &);
};