
#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup() {
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;

	// load background and sound
	background.load("images/starfield.jpg");
	movementSound.load("sounds/rocket-thrust.wav");	
	movementSound.setVolume(0.3 );
	movementSound.setLoop(true);
	explosionSound.load("sounds/explosion.mp3");
	explosionSound.setVolume(0.2);

	// free cam
	cam.setDistance(80);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();

	//cam tracking lander
	cam1.setNearClip(.1);
	cam1.setFov(65.5);
	cam1.setPosition(glm::vec3(lander.getPosition().x, lander.getPosition().y + 20, lander.getPosition().z));
	cam1.lookAt(lander.getPosition());

	//onboard camera
	cam2.setNearClip(.1);
	cam2.setFov(65.5);
	cam2.rotateDeg(10, glm::vec3(1, 0, 0));

	//top view cam
	top.setFov(65.5);
	top.setPosition(glm::vec3(lander.getPosition().x, lander.getPosition().y + 20, lander.getPosition().z));
	top.lookAt(lander.getPosition());

	theCam = &cam;

	ofEnableSmoothing();
	ofEnableDepthTest();

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.jpg")) {
		cout << "Particle Texture File: images/dot.jpg not found" << endl;
		ofExit();
	}

	// load the shader
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// setup rudimentary lighting 
	//
	keyLight.setup();
	keyLight.enable();
	keyLight.setAreaLight(1, 1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));

	keyLight.rotate(45, ofVec3f(0, 1, 0));
	keyLight.rotate(-45, ofVec3f(1, 0, 0));
	keyLight.setPosition(5, 5, 5);

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(15);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	fillLight.rotate(45, ofVec3f(1, 0, 0));
	fillLight.rotate(-45, ofVec3f(0, 1, 0));
	fillLight.setPosition(-5, 5, 5);

	rimLight.setup();
	rimLight.enable();
	rimLight.setScale(.05);
	rimLight.setAreaLight(1, 1);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	rimLight.rotate(180, ofVec3f(0, 1, 0));
	rimLight.setPosition(-5, 5, 5);

	initLightingAndMaterials();

	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);


	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	bHide = false;

	//  Create Octree for testing.
	//

	octree.create(mars.getMesh(0), 20);

	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));


	// set up lander and landing area
	//
	lander.loadModel("geo/lander.obj");
	lander.setScaleNormalization(false);
	lander.setPosition(50, 200, 30);
	bLanderLoaded = true;

	landerSys = new ParticleSystem();
	playerLander = new Particle();
	playerLander->lifespan = -1;
	landerSys->add(*playerLander);

	gravityForce = new GravityForce(ofVec3f(0, -2, 0));
	thrustForce = new ThrustForce(ofVec3f(0, 0, 0));
	landerSys->addForce(gravityForce);
	landerSys->addForce(thrustForce);

	landingBox = Box(Vector3(-6, 0, -6), Vector3(6, 0.5, 6));

	// particle emitter for rocket exhaust
	rocketExhaust.setEmitterType(DiscEmitter);
	rocketExhaust.setPosition(landerSys->particles[0].position);
	rocketExhaust.setVelocity(ofVec3f(0, -10, 0));
	rocketExhaust.setOneShot(true);
	rocketExhaust.setGroupSize(50);
	rocketExhaust.setLifespan(0.5);

	// particle emitter for explosion
	explosion.setEmitterType(RadialEmitter);
	impulseRadialForce = new ImpulseRadialForce(2000);
	explosion.sys->addForce(impulseRadialForce);
	explosion.setPosition(landerSys->particles[0].position);
	explosion.setOneShot(true);
	explosion.setGroupSize(100);
	explosion.setVelocity(ofVec3f(0, 0, 0));
	explosion.setLifespan(1);
	explosion.setParticleRadius(0.2);
}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	if (rocketExhaust.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < rocketExhaust.sys->particles.size(); i++) {
		points.push_back(rocketExhaust.sys->particles[i].position);
		sizes.push_back(ofVec3f(25));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// shader for explosion using same concept as loadVbo()
//
void ofApp::loadExplosionVbo() {
	if (explosion.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < explosion.sys->particles.size(); i++) {
		points.push_back(explosion.sys->particles[i].position);
		sizes.push_back(ofVec3f(25));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	explodeVbo.clear();
	explodeVbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	explodeVbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	ofSeedRandom();

	// update camera 
	cam1.setPosition(lander.getPosition().x, lander.getPosition().y + 25, lander.getPosition().z + 25);
	cam1.lookAt(lander.getPosition());
	cam2.setPosition(glm::vec3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z));
	cam2.lookAt(lander.getPosition());
	top.setPosition(lander.getPosition().x, lander.getPosition().y + 25, lander.getPosition().z);
	top.lookAt(lander.getPosition());

	// update lander
	if (bLanderLoaded && !bLanderSelected) {
		if (fuel <= 0) {
			fuel = 0;
			bFuel = false;
		}
		landerSys->particles[0].position = lander.getPosition();
		if (checkCollisions()) {
			bLanded = true;
			float restitution = 0.6;
			landerSys->particles[0].velocity.y *= -restitution;
			// lander has landed
			if (bLanded) {
				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				// landed in landing area
				if (bounds.overlap(landingBox)) {
					landedInBox = true;
					if (landerSys->particles[0].velocity.y < 3 && landerSys->particles[0].velocity.y > 0) {
						softLanding = true;
						finalScore = fuel + 100;
						// debug statement cout << "it works" << endl;
					}
				}
				else {
					landedOutsideBox = true;
				}
			}

			// lander going too fast
			if (landerSys->particles[0].velocity.y > 5) {
				// lander explodes
				impulseForce = new ImpulseForce(ofVec3f(0, 3000, 0));
				impulseForce->applyOnce = true;
				landerSys->addForce(impulseForce);
				explosion.sys->reset();
				explosion.start();
				explosionSound.play(); 
				// debug statement cout << "explosion" << endl;
				gameOver = true;
			}

			// landing a little hard
			else if (landerSys->particles[0].velocity.y > 3 && landedInBox) {
				roughLanding = true;
				// debug statement cout << "hard landing" << endl;
				finalScore = (fuel / 100) + 75;
			}

		}

		// update particle system and emitters
		landerSys->update();
		lander.setPosition(landerSys->particles[0].position.x, landerSys->particles[0].position.y, landerSys->particles[0].position.z);
		lander.setRotation(0, landerSys->particles[0].rotation, 0, 1, 0);
		rocketExhaust.setPosition(landerSys->particles[0].position);
		rocketExhaust.update();
		explosion.setPosition(landerSys->particles[0].position);
		explosion.update();

		// update altitude of lander
		altitude = getAltitude();
	}
}
//--------------------------------------------------------------
void ofApp::draw() {
	loadVbo();
	loadExplosionVbo();
	ofBackground(ofColor::black);

	glDepthMask(false);
	ofSetColor(ofColor::white);
	background.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	if (!bHide) gui.draw();
	glDepthMask(true);

	theCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		ofNoFill();
		ofSetColor(ofColor::blue);
		Octree::drawBox(landingBox);

		// set up heading
		float angle = glm::radians(landerSys->particles[0].rotation);
		headingVector = glm::normalize(glm::vec3(-glm::sin(angle), 0, -glm::cos(angle)));

		ofFill();
		if (bLanderLoaded) {
			lander.drawFaces();
			rocketExhaust.draw();
			explosion.draw();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				ofNoFill();
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
	}
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}

	ofPopMatrix();
	theCam->end();

	glDepthMask(GL_FALSE);
	ofSetColor(255, 100, 90);

	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	// begin drawing in the camera
	//
	shader.begin();
	theCam->begin();

	// draw particle emitter here..
	//

	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)rocketExhaust.sys->particles.size());
	explodeVbo.draw(GL_POINTS, 0, (int)explosion.sys->particles.size());
	particleTex.unbind();

	//  end drawing in the camera
	// 
	theCam->end();
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofEnableAlphaBlending();

	// set back the depth mask
	//
	glDepthMask(GL_TRUE);

	// draw screen data
	//
	string altitudeText;
	altitudeText += "Current Altitude: " + to_string(altitude);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(altitudeText, ofGetWindowWidth() / 2 + 300, 40);

	string currentFuel;
	currentFuel += "Current Fuel: " + to_string(fuel) + " / 120 seconds";
	ofSetColor(ofColor::white);
	ofDrawBitmapString(currentFuel, ofGetWindowWidth() / 2 + 300, 60); 
		
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'r':
		cam.reset();
		cam.setPosition(ofVec3f(0, 0, 0));
		cam.setDistance(80);
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case ' ':
		gameOver = false;
		landedInBox = false;
		roughLanding = false;
		softLanding = false;
		bFuel = true;
		lander.setPosition(50, 200, 30);
		fuel = 120;
		break;
	case 'w':
		if (bFuel && !gameOver) {
			thrusterOn = true;
			fuel -= ofGetLastFrameTime();
			thrustForce->add(0.5 * ofVec3f(0, 1, 0));
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play();
			}
		}
		break;
	case 's':
		if (bFuel && !gameOver) {
			thrusterOn = true;
			fuel -= ofGetLastFrameTime();
			thrustForce->add(0.5 * ofVec3f(0, -1, 0));
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play();
			}
		}
		break;
	case 'q':
		if (bFuel && !gameOver) {
			landerSys->particles[0].rForce = 10;
		}
		break;
	case 'e':
		if (bFuel && !gameOver) {
			landerSys->particles[0].rForce = -10;
		}
		break;
	case OF_KEY_LEFT:
		if (bFuel && !gameOver) {
			thrusterOn = true;
			fuel -= ofGetLastFrameTime();
			thrustForce->add(0.5 * ofVec3f(-1, 0, 0));
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play(); 
			}
		}
		break;
		break;
	case OF_KEY_RIGHT:
		if (bFuel && !gameOver) {
			thrusterOn = true;
			fuel -= ofGetLastFrameTime();
			thrustForce->add(0.5 * ofVec3f(1, 0, 0));
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play();
			}
		}
		break;
		break;
	case OF_KEY_UP:
		thrusterOn = true;
		fuel -= ofGetLastFrameTime();
		if (bFuel && !gameOver) {
			thrustForce->add(0.5 * headingVector);
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play();
			}
		}
		break;
		break;
	case OF_KEY_DOWN:
		thrusterOn = true;
		fuel -= ofGetLastFrameTime();
		if (bFuel && !gameOver) {
			thrustForce->add(0.5 * -headingVector);
			rocketExhaust.start();
			if (!movementSound.isPlaying()) {
				movementSound.play();
			}
		}
		break;
		break;
	case '1':
		theCam = &cam;
		break;
	case '2':
		theCam = &cam1;
		break;
	case '3':
		theCam = &cam2;
		break;
	case '4':
		theCam = &top;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
	rocketExhaust.stop();
	movementSound.stop();
	switch (key) {
	case 'w':
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case 's':
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case 'q':
		if (bFuel && !gameOver) {
			landerSys->particles[0].rForce = 0;
		}
		break;
	case 'e':
		if (bFuel && !gameOver) {
			landerSys->particles[0].rForce = 0;
		}
		break;
	case OF_KEY_LEFT:
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case OF_KEY_RIGHT:
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case OF_KEY_UP:
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case OF_KEY_DOWN:
		if (bFuel && !gameOver) {
			thrustForce->set(ofVec3f(0, 0, 0));
		}
		break;
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {


}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f& pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));
	//time to search data with ray intersection in microseconds
	float start = ofGetElapsedTimeMicros();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	float finish = ofGetElapsedTimeMicros() - start;
	cout << "Finished intersection\nIntersection time: " << finish << " microseconds" << endl;
	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);


		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{ 5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
		//		lander.setScale(.1, .1, .1);
			//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f& point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

float ofApp::getAltitude() {
	Ray aRay = Ray(Vector3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z), Vector3(0, -1, 0));
	octree.intersect(aRay, octree.root, selectedNode);
	glm::vec3 p = mars.getMesh(0).getVertex(selectedNode.points[0]);
	return glm::length(p - lander.getPosition());
}

bool ofApp::checkCollisions() {
	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();

	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	// collision detection with terrain and lander
	for (int i = 0; i < colBoxList.size(); i++) {
		if (bounds.overlap(colBoxList[i])) {
			//take top of terrain
			ofVec3f topOfTerrain = ofVec3f(colBoxList[i].parameters[0].x(), colBoxList[i].parameters[1].y(), colBoxList[i].parameters[0].z());
			//take bottom of lander
			ofVec3f bottomOfLander = ofVec3f(landerSys->particles[0].position.x, landerSys->particles[0].position.y, landerSys->particles[0].position.z);
			// calculate collision and also adjust so lander does not go through terrain
			float collision = bottomOfLander.y - topOfTerrain.y;
			landerSys->particles[0].position.y -= collision - 0.03;

			// debug satatement cout << landerSys->particles[0].velocity.y << endl;
			return true;
		}
	}
	return false;
}