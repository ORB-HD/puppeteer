/*
 * QtGLAppBase - Simple Qt Application to get started with OpenGL stuff.
 *
 * Copyright (c) 2011-2012 Martin Felis <martin.felis@iwr.uni-heidelberg.de>
 *
 * Licensed under the MIT license. See LICENSE for more details.
 */

#include <QtGui> 
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QProgressDialog>
#include <QRegExp>
#include <QRegExpValidator>

#include "GLWidget.h" 
#include "QtGLBaseApp.h"
#include "QtVariantPropertyManager"

#include "Scene.h"
#include "MarkerModel.h"

#include <sys/time.h>
#include <ctime>

#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <fstream>

using namespace std;

using namespace SimpleMath::GL;

const double TimeLineDuration = 1000.;

std::string vec3_to_string (const Vector3f vec3, unsigned int digits = 2) {
	stringstream vec_stream ("");
	vec_stream << std::fixed << std::setprecision(digits) << vec3[0] << ", " << vec3[1] << ", " << vec3[2];

	return vec_stream.str();
}

Vector3f string_to_vec3 (const std::string &vec3_string) {
	Vector3f result;

	unsigned int token_start = 0;
	unsigned int token_end = vec3_string.find(",");
	for (unsigned int i = 0; i < 3; i++) {
		string token = vec3_string.substr (token_start, token_end - token_start);

		result[i] = static_cast<float>(atof(token.c_str()));

		token_start = token_end + 1;
		token_end = vec3_string.find (", ", token_start);
	}

	return result;
}

QtGLBaseApp::~QtGLBaseApp() {
	if (scene)
		delete scene;

	scene = NULL;
}

QtGLBaseApp::QtGLBaseApp(QWidget *parent)
{
	setupUi(this); // this sets up GUI

	// create Scene
	scene = new Scene;	
	scene->init();
	glWidget->setScene (scene);

	// marker model
	markerModel = new MarkerModel(scene);

	drawTimer = new QTimer (this);
	drawTimer->setSingleShot(false);
	drawTimer->start(20);

	checkBoxDrawBaseAxes->setChecked (glWidget->draw_base_axes);
	checkBoxDrawGrid->setChecked (glWidget->draw_grid);

	// camera controls
	QRegExp	vector3_expr ("^\\s*-?\\d*(\\.|\\.\\d+)?\\s*,\\s*-?\\d*(\\.|\\.\\d+)?\\s*,\\s*-?\\d*(\\.|\\.\\d+)?\\s*$");
	QRegExpValidator *coord_validator_eye = new QRegExpValidator (vector3_expr, lineEditCameraEye);
	QRegExpValidator *coord_validator_center = new QRegExpValidator (vector3_expr, lineEditCameraCenter);
	lineEditCameraEye->setValidator (coord_validator_eye);
	lineEditCameraCenter->setValidator (coord_validator_center);

	dockCameraControls->setVisible(false);

	// view stettings
	connect (checkBoxDrawBaseAxes, SIGNAL (toggled(bool)), glWidget, SLOT (toggle_draw_base_axes(bool)));
	connect (checkBoxDrawGrid, SIGNAL (toggled(bool)), glWidget, SLOT (toggle_draw_grid(bool)));
	//connect (checkBoxDrawShadows, SIGNAL (toggled(bool)), glWidget, SLOT (toggle_draw_shadows(bool)));

	connect (actionFrontView, SIGNAL (triggered()), glWidget, SLOT (set_front_view()));
	connect (actionSideView, SIGNAL (triggered()), glWidget, SLOT (set_side_view()));
	connect (actionTopView, SIGNAL (triggered()), glWidget, SLOT (set_top_view()));
	connect (actionToggleOrthographic, SIGNAL (toggled(bool)), glWidget, SLOT (toggle_draw_orthographic(bool)));

	// actionQuit() makes sure to set the settings before we quit
	connect (actionQuit, SIGNAL( triggered() ), this, SLOT( quitApplication() ));
	connect (pushButtonUpdateCamera, SIGNAL (clicked()), this, SLOT (updateCamera()));

	// call the drawing function
	connect (drawTimer, SIGNAL(timeout()), glWidget, SLOT (updateGL()));

	// object selection
	connect (glWidget, SIGNAL(object_selected(int)), this, SLOT (updateWidgetsFromObject(int)));

	// object widgets
	QRegExpValidator *position_validator = new QRegExpValidator (vector3_expr, lineEditObjectPosition);
	lineEditObjectPosition->setValidator(position_validator);
	connect (lineEditObjectPosition, SIGNAL(editingFinished()), this, SLOT(updateObjectFromWidget()));

	QRegExpValidator *rotation_validator = new QRegExpValidator (vector3_expr, lineEditObjectRotation);
	lineEditObjectRotation->setValidator(rotation_validator);
	connect (lineEditObjectRotation, SIGNAL(editingFinished()), this, SLOT(updateObjectFromWidget()));

	QRegExpValidator *scale_validator = new QRegExpValidator (vector3_expr, lineEditObjectScaling);
	lineEditObjectScaling->setValidator(scale_validator);
	connect (lineEditObjectScaling, SIGNAL(editingFinished()), this, SLOT(updateObjectFromWidget()));

	// property browser test
	variantManager = new QtVariantPropertyManager ();
	variantEditorFactory = new QtVariantEditorFactory();
	propertiesBrowser->setFactoryForManager (variantManager, variantEditorFactory);

	connect (variantManager, SIGNAL (valueChanged (QtProperty *, const QVariant &)), this, SLOT (propertyChanged(QtProperty *, const QVariant &)));
}

void print_usage(const char* execname) {
	cout << "Usage: " << execname << " [modelfile.lua]" << endl;
}

bool QtGLBaseApp::parseArgs(int argc, char* argv[]) {
	if (argc == 1)
		return true;

	if (argc == 2) {
		loadModelFile (argv[1]);
	} else {
		print_usage (argv[0]);
		return false;
	}

	return true;
}

bool QtGLBaseApp::loadModelFile (const char* filename) {
	assert (markerModel);
	return markerModel->loadFromFile (filename);
}

void QtGLBaseApp::cameraChanged() {
	Vector3f center = glWidget->getCameraPoi();	
	Vector3f eye = glWidget->getCameraEye();	

	unsigned int digits = 2;

	stringstream center_stream ("");
	center_stream << std::fixed << std::setprecision(digits) << center[0] << ", " << center[1] << ", " << center[2];

	stringstream eye_stream ("");
	eye_stream << std::fixed << std::setprecision(digits) << eye[0] << ", " << eye[1] << ", " << eye[2];

	lineEditCameraEye->setText (eye_stream.str().c_str());
	lineEditCameraCenter->setText (center_stream.str().c_str());
}

Vector3f parse_vec3_string (const std::string vec3_string) {
	Vector3f result;

	unsigned int token_start = 0;
	unsigned int token_end = vec3_string.find(",");
	for (unsigned int i = 0; i < 3; i++) {
		string token = vec3_string.substr (token_start, token_end - token_start);

		result[i] = static_cast<float>(atof(token.c_str()));

		token_start = token_end + 1;
		token_end = vec3_string.find (", ", token_start);
	}

//	cout << "Parsed '" << vec3_string << "' to " << result.transpose() << endl;

	return result;
}

void QtGLBaseApp::updateCamera() {
	string center_string = lineEditCameraCenter->text().toStdString();
	Vector3f poi = parse_vec3_string (center_string);

	string eye_string = lineEditCameraEye->text().toStdString();
	Vector3f eye = parse_vec3_string (eye_string);

	glWidget->setCameraPoi(poi);
	glWidget->setCameraEye(eye);
}

void QtGLBaseApp::quitApplication() {
	qApp->quit();
}

void QtGLBaseApp::propertyChanged(QtProperty *property, const QVariant &variant) {
	
	qDebug() << "something changed!";
}

void QtGLBaseApp::updateWidgetsFromObject (int object_id) {
	if (object_id < 0) {
		lineEditObjectPosition->setText ("");
		return;
	}

	if (markerModel->isJointObject(object_id)) {
		qDebug() << "clicked on joint!";
		lineEditObjectPosition->setText ("");
		return;
	}

	lineEditObjectPosition->setText (vec3_to_string (scene->objects[object_id].transformation.translation).c_str());

	Vector3f zyx_rotation = scene->objects[object_id].transformation.rotation.toEulerZYX() * 180.f / static_cast<float>(M_PI);

	lineEditObjectRotation->setText (vec3_to_string (zyx_rotation).c_str());

	Vector3f scaling = scene->objects[object_id].transformation.scaling;

	lineEditObjectScaling->setText (vec3_to_string (scaling).c_str());

	variantManager->clear();

	Vector3f position = scene->objects[object_id].transformation.translation;

	QtVariantProperty *pos_x = variantManager->addProperty(QVariant::Double, "Position X");
	pos_x->setAttribute ("minimum", -10.);
	pos_x->setAttribute ("maximum", 10.);
	pos_x->setValue (position[0]);

	propertiesBrowser->addProperty(pos_x);

	QtVariantProperty *pos_y = variantManager->addProperty(QVariant::Double, "Position Y");
	pos_y->setAttribute ("minimum", -10.);
	pos_y->setAttribute ("maximum", 10.);
	pos_y->setValue (position[1]);

	propertiesBrowser->addProperty(pos_y);

	QtVariantProperty *pos_z = variantManager->addProperty(QVariant::Double, "Position Z");
	pos_z->setAttribute ("minimum", -10.);
	pos_z->setAttribute ("maximum", 10.);
	pos_z->setValue (position[2]);

	propertiesBrowser->addProperty(pos_z);

}

void QtGLBaseApp::updateObjectFromWidget () {
	if (scene->selectedObjectId < 0)
		return;

	Vector3f position = string_to_vec3 (lineEditObjectPosition->text().toStdString());

	Vector3f zyx_rotation = string_to_vec3 (lineEditObjectRotation->text().toStdString()) * M_PI / 180.f;
	Quaternion rotation = Quaternion::fromEulerZYX (zyx_rotation);

	Vector3f scaling = string_to_vec3 (lineEditObjectScaling->text().toStdString());

	scene->objects[scene->selectedObjectId].transformation.translation = position;
	scene->objects[scene->selectedObjectId].transformation.rotation = rotation;
	scene->objects[scene->selectedObjectId].transformation.scaling = scaling;
}

