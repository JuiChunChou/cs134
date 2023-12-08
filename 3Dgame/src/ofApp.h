#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include <glm/gtx/intersect.hpp>


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent2(ofDragInfo dragInfo);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void loadVbo();
	void loadExplosionVbo();
	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
	void toggleSelectTerrain();
	void setCameraTarget();
	bool checkCollisions();
	bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point);
	bool raySelectWithOctree(ofVec3f& pointRet);
	glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p, glm::vec3 n);
	float getAltitude();

	ofEasyCam cam;
	ofCamera top, cam1, cam2;
	ofCamera* theCam;
	ofxAssimpModelLoader mars, lander;
	ofLight light;
	Box boundingBox, landerBounds;
	Box landingBox;
	Box testBox;
	vector<Box> colBoxList;
	bool bLanderSelected = false;
	Octree octree;
	TreeNode selectedNode;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;


	ofxIntSlider numLevels;
	ofxPanel gui;
	ofxToggle drawHeading;

	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bWireframe;
	bool bDisplayPoints;
	bool bPointSelected;
	bool bHide;
	bool pointSelected = false;
	bool bDisplayLeafNodes = false;
	bool bDisplayOctree = false;
	bool bDisplayBBoxes = false;

	bool bLanderLoaded;
	bool bTerrainSelected;

	ofVec3f selectedPoint;
	ofVec3f intersectPoint;

	vector<Box> bboxList;

	const float selectionRange = 4.0;

	bool gameStarted;
	bool gameOver;
	bool bFuel = true;
	bool bLanded = false;
	bool landedInBox = false;
	bool landedOutsideBox = false;
	bool softLanding = false;
	bool roughLanding = false;
	bool thrusterOn = false;
	float altitude;
	float finalScore;
	float angle;
	float fuel = 10;


	glm::vec3 headingVector;

	Particle* playerLander;
	ParticleSystem* landerSys;
	GravityForce* gravityForce;
	ThrustForce* thrustForce;
	ImpulseForce* impulseForce;
	ImpulseRadialForce* impulseRadialForce;
	ParticleEmitter explosion, rocketExhaust;
	ofLight keyLight, fillLight, rimLight;
	ofSoundPlayer movementSound, explosionSound;
	ofImage background;
	ofTexture particleTex;
	ofVbo vbo, explodeVbo;
	ofShader shader;
};