#include "Scene.h"
#include "MarkerData.h"
#include "c3dfile.h"

using namespace std;

MarkerData::~MarkerData() {
	if (c3dfile) {
		delete c3dfile;
		c3dfile = NULL;
	}
	for (int i = 0; i < markers.size(); i++) {
		scene->destroyObject<MarkerObject>(markers[i]);
	}
}

bool MarkerData::loadFromFile(const char *filename) {
	assert (scene);

	if (c3dfile) {
		delete c3dfile;
		markers.clear();
	}

	c3dfile = new C3DFile;
	if (!c3dfile->load (filename)) {
		cerr << "Error loading marker data from file '" << filename << "'!" << endl;
		abort();
		return false;
	}

	currentFrame = getFirstFrame();

	enableMarker ("LFHD", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("LBHD", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("RFHD", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("RBHD", Vector3f (0.f, 0.f, 1.f));

	enableMarker ("LASI", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("LPSI", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("RASI", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("RPSI", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("C7",   Vector3f (0.f, 0.f, 1.f));
	enableMarker ("STRN", Vector3f (0.f, 0.f, 1.f));
	enableMarker ("T10",  Vector3f (0.f, 0.f, 1.f));
	
	enableMarker ("RSHO", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RUPA", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RELB", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RWRA", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RWRB", Vector3f (1.f, 0.f, 0.f));

	enableMarker ("LSHO", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LUPA", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LELB", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LWRA", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LWRB", Vector3f (0.f, 1.f, 0.f));

	enableMarker ("RTHI", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RTIB", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RKNE", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RANK", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RHEE", Vector3f (1.f, 0.f, 0.f));
	enableMarker ("RTOE", Vector3f (1.f, 0.f, 0.f));

	enableMarker ("LTHI", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LTIB", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LKNE", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LANK", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LHEE", Vector3f (0.f, 1.f, 0.f));
	enableMarker ("LTOE", Vector3f (0.f, 1.f, 0.f));

	return true;
}

void MarkerData::enableMarker (const char* marker_name, const Vector3f &color) {
	assert (c3dfile);
	assert (scene);

	MarkerObject* scene_marker = scene->createObject<MarkerObject>();
	scene_marker->color.block<3,1>(0,0) = color;

	Vector3f position = getMarkerCurrentPosition(marker_name);
	scene_marker->transformation.translation = position;
	scene_marker->mesh = CreateUVSphere (4, 8);
	scene_marker->transformation.scaling = Vector3f (0.02f, 0.02f, 0.02f);
	scene_marker->noDepthTest = true;
	scene_marker->markerName = marker_name;

	markers.push_back (scene_marker);
}

Vector3f MarkerData::getMarkerCurrentPosition(const char * marker_name) {
	FloatMarkerData marker_traj = c3dfile->getMarkerTrajectories (marker_name);
	
	int index = currentFrame - getFirstFrame();

	return Vector3f (marker_traj.x[index], marker_traj.y[index], marker_traj.z[index]) * 1.0e-3;
}

int MarkerData::getFirstFrame () {
	assert (c3dfile);

	return static_cast<int>(c3dfile->header.first_frame);
}

int MarkerData::getLastFrame () {
	assert (c3dfile);

	return static_cast<int>(c3dfile->header.last_frame);
}

void MarkerData::setCurrentFrameNumber (int frame_number) {
	assert (frame_number >= getFirstFrame());
	assert (frame_number <= getLastFrame());

	currentFrame = frame_number;

	updateMarkerSceneObjects();
}

void MarkerData::updateMarkerSceneObjects() {
	for (size_t i = 0; i < markers.size(); i++) {
		Vector3f position = getMarkerCurrentPosition(markers[i]->markerName.c_str());
		markers[i]->transformation.translation = position;
	}
}
