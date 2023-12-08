

#include "Octree.h"
 


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces,
	Box & box, vector<int> & facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p,3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	float timeToBuild = ofGetElapsedTimeMillis();
	// initialize octree structure
	//
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}
	else {
		// need to load face vertices here
		//
	}

	// recursively buid octree
	//
	level++;
    subdivide(mesh, root, numLevels, level);

	//time to build octree
	cout << "Time to build octree: " << ofGetElapsedTimeMillis() - timeToBuild << "ms" << endl;
}
      
             
void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;

	// subdvide algorithm implemented here
	vector<Box> tempBox;
	subDivideBox8(node.box, tempBox);
	
	for (int i = 0; i < tempBox.size(); i++) {
		vector<int> pointsRtn;
		getMeshPointsInBox(mesh, node.points, tempBox[i], pointsRtn);
		if (pointsRtn.size() > 0) {
			TreeNode& t = TreeNode();
			t.box = tempBox[i];
			t.points = pointsRtn;
			node.children.push_back(t);
			if (pointsRtn.size() > 1) {
				subdivide(mesh, node.children.back(), numLevels, level);
				level++;
			}
			else {
				numLeaf++;
			}
		}
	}
}

// Implement functions below for Homework project
//

//using recursion
bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	bool intersects = false;
	if (node.points.size() == 1) {
		intersects = node.box.intersect(ray, -1000, 1000);
		if (intersects)
		{
			nodeRtn = node;
		}
	}
	else {
		for (int i = 0; i < node.children.size(); i++) { 
			if (node.children[i].box.intersect(ray, -1000, 1000)) {
				if (intersect(ray, node.children[i], nodeRtn)) {
					intersects = true;
					break;
				}
			}
		}
	}
	return intersects;
}


//using recursion
bool Octree::intersect(const Box &box, TreeNode & node, vector<Box> & boxListRtn) {
	bool intersects = false;
	//use the overlap method built in box.h
	if (node.box.overlap(box)) {
		if (node.points.size() == 1) {
			boxListRtn.push_back(node.box);
			intersects = true;
			return intersects;
		}
		for (int i = 0; i < node.children.size(); i++) {
			intersect(box, node.children[i], boxListRtn);
		}
	}
}

//referred to octree readme
void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	ofSetColor(colors[level]);
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// Optional
//
void Octree::drawLeafNodes(TreeNode & node) {

}




