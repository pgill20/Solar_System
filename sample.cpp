#include <stdio.h>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#endif

#include "glew.h"
#include <OpenGl/gl.h>
#include <OpenGl/glu.h>
#include "glut.h"

const float ZNEAR = 10000;
const float ZFAR = 10000000;

float Time;
bool Freeze;
bool Distorted;
int ObjectTexture;
bool Light2On;

enum views
{
	OUTSIDEVIEW,
	EARTHVIEW,
	MOONVIEW,
	OUT
};
enum views WhichView;

const int MAXIMUM_TIME_SECONDS = 10 * 60; // I decided to use 10 minutes
const int MAXIMUM_TIME_MILLISECONDS = 1000 * MAXIMUM_TIME_SECONDS;
const float ONE_FULL_TURN = 2. * M_PI; // this is 2π instead of 360° because glm uses radians

GLuint Tex0, Tex1, Tex2, Tex3, Tex4, Tex5, Tex6, Tex7, Tex8, Tex9;
unsigned char *TextureArray0, *TextureArray1, *TextureArray2, *TextureArray3, *TextureArray4, *TextureArray5, *TextureArray6, *TextureArray7, *TextureArray8, *TextureArray9;

void OsuSphere(float, int, int);
void drawSun(int);

float *Array3(float a, float b, float c);
void SetPointLight(int ilight, float x, float y, float z, float r, float g, float b);
void SetMaterial(float r, float g, float b, float shininess);
float *MulArray3(float factor, float array0[3]);

void LatLngToXYZ(float lat, float lng, float rad, glm::vec3 *xyzp)
{
	lat = glm::radians(lat);
	lng = glm::radians(lng);
	xyzp->y = rad * sin(lat);
	float xz = cos(lat);
	xyzp->x = rad * xz * cos(lng);
	xyzp->z = rad * xz * sin(lng);
}

void SetViewingFromLatLng(float eyeLat, float eyeLng, float lookLat, float lookLng, float rad, glm::vec4 *eyep, glm::vec4 *lookp)
{
	glm::vec3 eye, look;
	LatLngToXYZ(eyeLat, eyeLng, rad, &eye);
	LatLngToXYZ(lookLat, lookLng, rad, &look);
	glm::vec3 up = glm::normalize(eye); // only true for spheres !!
	glm::vec3 eyeToLook = look - eye;
	glm::vec3 parallelToUp = glm::dot(up, eyeToLook) * eyeToLook;
	eyeToLook = eyeToLook - parallelToUp;
	*eyep = glm::vec4(eye, 1.);
	*lookp = glm::vec4(eye + eyeToLook, 1.);
}

const float EARTH_RADIUS_MILES = 2;
const float EARTH_ORBITAL_RADIUS_MILES = 19.29;
const float EARTH_ORBIT_TIME_DAYS = 0.0000003653;
const float EARTH_ORBIT_TIME_HOURS = EARTH_ORBIT_TIME_DAYS * 24.;
const float EARTH_ORBIT_TIME_SECONDS = EARTH_ORBIT_TIME_HOURS * 60. * 60.;
const float EARTH_SPIN_TIME_DAYS = 0.00009971;
const float EARTH_SPIN_TIME_HOURS = EARTH_SPIN_TIME_DAYS * 24.;
const float EARTH_SPIN_TIME_SECONDS = EARTH_SPIN_TIME_HOURS * 60. * 60.;

const float MOON_RADIUS_MILES = 0.55;
const float MOON_ORBITAL_RADIUS_MILES = 2.38900;
const float MOON_ORBIT_TIME_DAYS = 0.0000000273;
const float MOON_ORBIT_TIME_HOURS = MOON_ORBIT_TIME_DAYS * 24.;
const float MOON_ORBIT_TIME_SECONDS = MOON_ORBIT_TIME_HOURS * 60. * 60.;
const float MOON_SPIN_TIME_DAYS = MOON_ORBIT_TIME_DAYS;
const float MOON_SPIN_TIME_HOURS = MOON_SPIN_TIME_DAYS * 24.;
const float MOON_SPIN_TIME_SECONDS = MOON_SPIN_TIME_HOURS * 60. * 60.;

glm::mat4
MakeMercuryMatrix()
{
	float mercurySpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 0.017241379;
	float mercuryOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 4.147727273;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, mercuryOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 0.709175739, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, mercurySpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeVenusMatrix()
{
	float venusSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 0.004115226;
	float venusOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 1.622222222;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, venusOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 0.866770347, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, venusSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeEarthMatrix()
{
	float earthSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN;
	float earthOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, earthOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, earthSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeMoonMatrix()
{
	float moonSpinAngle = Time * MOON_SPIN_TIME_SECONDS * ONE_FULL_TURN;
	float moonOrbitAngle = Time * MOON_ORBIT_TIME_SECONDS * ONE_FULL_TURN;
	float earthOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 5. */ glm::mat4 erorbity = glm::rotate(identity, earthOrbitAngle, yaxis);
	/* 4. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES, 0., 0.));
	/* 3. */ glm::mat4 mrorbity = glm::rotate(identity, moonOrbitAngle, yaxis);
	/* 2. */ glm::mat4 mtransx = glm::translate(identity, glm::vec3(MOON_ORBITAL_RADIUS_MILES, 0., 0.));
	/* 1. */ glm::mat4 mrspiny = glm::rotate(identity, moonSpinAngle, yaxis);
	return erorbity * etransx * mrorbity * mtransx * mrspiny; // 5 * 4 * 3 * 2 * 1
}

glm::mat4
MakeMarsMatrix()
{
	float marsSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 1.;
	float marsOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 0.531295488;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, marsOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 1.296008294, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, marsSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeJupiterMatrix()
{
	float jupiterSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 2.380952381;
	float jupiterOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 0.084237249;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, jupiterOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 1.814411612, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, jupiterSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeSaturnMatrix()
{
	float saturnSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 2.272727273;
	float saturnOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 0.033925086;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, saturnOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 3.110419907, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, saturnSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeUranusMatrix()
{
	float uranusSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 1.408450704;
	float uranusOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 0.0118942872;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, uranusOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 3.888024883, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, uranusSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

glm::mat4
MakeNeptuneMatrix()
{
	float neptuneSpinAngle = Time * EARTH_SPIN_TIME_SECONDS * ONE_FULL_TURN * 1.492537313;
	float neptuneOrbitAngle = Time * EARTH_ORBIT_TIME_SECONDS * ONE_FULL_TURN * 0.00606413;
	glm::mat4 identity = glm::mat4(1.);
	glm::vec3 yaxis = glm::vec3(0., 1., 0.);
	/* 3. */ glm::mat4 erorbity = glm::rotate(identity, neptuneOrbitAngle, yaxis);
	/* 2. */ glm::mat4 etransx = glm::translate(identity, glm::vec3(EARTH_ORBITAL_RADIUS_MILES * 5.184033178, 0., 0.)); /* 1. */
	glm::mat4 erspiny = glm::rotate(identity, neptuneSpinAngle, yaxis);
	return erorbity * etransx * erspiny; // 3 * 2 * 1
}

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a Solar System with views from Earth and Moon.
//
//	Author:			Paramvir Gill

// title of these windows:

const char *WINDOWTITLE = "OpenGL / Solar System -- Paramvir Gill";
const char *GLUITITLE = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 1000;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[] = {0., 0., 0., 1.};

// line width for the axes:

const GLfloat AXES_WIDTH = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char *ColorNames[] =
	{
		(char *)"Red",
		(char *)"Yellow",
		(char *)"Green",
		(char *)"Cyan",
		(char *)"Blue",
		(char *)"Magenta",
		(char *)"White",
		(char *)"Black"};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
	{
		{1., 0., 0.}, // red
		{1., 1., 0.}, // yellow
		{0., 1., 0.}, // green
		{0., 1., 1.}, // cyan
		{0., 0., 1.}, // blue
		{1., 0., 1.}, // magenta
		{1., 1., 1.}, // white
		{0., 0., 0.}, // black
};

GLfloat White[] = {1., 1., 1., 1.};

// fog parameters:

const GLfloat FOGCOLOR[4] = {.0f, .0f, .0f, 1.f};
const GLenum FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;

// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// non-constant global variables:

int ActiveButton;	 // current button that is down
GLuint AxesList;	 // list to hold the axes
int AxesOn;			 // != 0 means to draw the axes
int DebugOn;		 // != 0 means to print debugging info
int DepthCueOn;		 // != 0 means to use intensity depth cueing
int DepthBufferOn;	 // != 0 means to use the z-buffer
int DepthFightingOn; // != 0 means to force the creation of z-fighting
GLuint BoxList;		 // object display list
int MainWindow;		 // window id for main graphics window
float Scale;		 // scaling factor
int ShadowsOn;		 // != 0 means to turn shadows on
int WhichColor;		 // index into Colors[ ]
int WhichProjection; // ORTHO or PERSP
int Xmouse, Ymouse;	 // mouse values
float Xrot, Yrot;	 // rotation angles in degrees

// function prototypes:

void Animate();
void Display();
void DoAxesMenu(int);
void DoColorMenu(int);
void DoDepthBufferMenu(int);
void DoDepthFightingMenu(int);
void DoDepthMenu(int);
void DoDebugMenu(int);
void DoMainMenu(int);
void DoProjectMenu(int);
void DoRasterString(float, float, float, char *);
void DoStrokeString(float, float, float, float, char *);
float ElapsedSeconds();
void InitGraphics();
void InitLists();
void InitMenus();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);
void Reset();
void Resize(int, int);
void Visibility(int);

void Axes(float);

unsigned char *BmpToTexture(char *, int *, int *);
int ReadInt(FILE *);
short ReadShort(FILE *);

void HsvRgb(float[3], float[3]);

void Cross(float[3], float[3], float[3]);
float Dot(float[3], float[3]);
float Unit(float[3], float[3]);

// main program:

int main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display structures that will not change:

	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}

// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	int ms = glutGet(GLUT_ELAPSED_TIME); // milliseconds
	ms %= MAXIMUM_TIME_MILLISECONDS;	 // [ 0, MAXIMUM_TIME_MILLISECONDS-1 ]
	Time = (float)ms / 1000.f;			 // seconds

	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// draw the complete scene:

void Display()
{
	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);

	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif

	// specify shading to be flat:

	glShadeModel(GL_FLAT);

	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy; // minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);

	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:

	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);

	// set the fog parameters:

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable(GL_NORMALIZE);

	// draw the box object by calling up its display list:

	// glCallList(BoxList);
	glm::mat4 e, m;
	glm::vec4 eyePos = glm::vec4(0., 0., 0., 1.);
	glm::vec4 lookPos = glm::vec4(0., 0., 0., 1.);
	glm::vec4 upVec = glm::vec4(0., 0., 0., 0.); // vectors don’t get translations

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	switch (WhichView)
	{
	case OUTSIDEVIEW: // 1st way to set gluLookAt( )
		gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);
		glRotatef((GLfloat)Yrot, 0., 1., 0.);
		glRotatef((GLfloat)Xrot, 1., 0., 0.);
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		glScalef(Scale, Scale, Scale);
		break;

	case EARTHVIEW: // 2nd way to set gluLookAt( )
		e = MakeEarthMatrix();
		SetViewingFromLatLng(0., 0., 0., -10., EARTH_RADIUS_MILES, &eyePos, &lookPos);
		e = MakeEarthMatrix();
		upVec = glm::vec4(glm::normalize(glm::vec3(eyePos.x)), 0.);
		eyePos = e * eyePos;
		lookPos = e * lookPos;
		upVec = e * upVec;
		// glTranslatef(0., 0., -6.f * ZNEAR);
		gluLookAt(eyePos.x, eyePos.y, eyePos.z, lookPos.x, lookPos.y, lookPos.z, upVec.x, upVec.y, upVec.z);
		break;

	case MOONVIEW: // 3rd way to set gluLookAt( )
		m = MakeMoonMatrix();
		SetViewingFromLatLng(0., 0., 0., -10., MOON_RADIUS_MILES, &eyePos, &lookPos);
		m = MakeMoonMatrix();
		upVec = glm::vec4(glm::normalize(glm::vec3(eyePos.x)), 0.);
		eyePos = m * eyePos;
		lookPos = m * lookPos;
		upVec = m * upVec;
		// glTranslatef(0., 0., -4.f * ZNEAR);
		gluLookAt(eyePos.x, eyePos.y, eyePos.z, lookPos.x, lookPos.y, lookPos.z, upVec.x, upVec.y, upVec.z);
		break;

	case OUT:
		gluLookAt(0.f, 100.f, 150.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		break;
	}

	glPushMatrix();

	glEnable(GL_TEXTURE_2D);

	// Sun
	glBindTexture(GL_TEXTURE_2D, Tex0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPushMatrix();
	if (ObjectTexture == 0)
	{
		glDisable(GL_TEXTURE_2D);
		drawSun(ObjectTexture);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		if (Light2On)
		{
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT2);
			SetPointLight(GL_LIGHT2, 0, 0, 0, 1, 1, 1);
			drawSun(ObjectTexture);
		}
		else
		{
			glDisable(GL_LIGHT2);
			drawSun(ObjectTexture);
		}
	}

	glPopMatrix();

	glm::mat4 mercury = MakeMercuryMatrix();
	glm::mat4 venus = MakeVenusMatrix();
	glm::mat4 earth = MakeEarthMatrix();
	glm::mat4 moon = MakeMoonMatrix();
	glm::mat4 mars = MakeMarsMatrix();
	glm::mat4 saturn = MakeSaturnMatrix();
	glm::mat4 jupiter = MakeJupiterMatrix();
	glm::mat4 uranus = MakeUranusMatrix();
	glm::mat4 neptune = MakeNeptuneMatrix();

	glEnable(GL_TEXTURE_2D);

	// DRAWING ALL PLANETS AROUND SUN:

	// Draw Mercury
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex3);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(mercury));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(0.75, 50, 50);
	glPopMatrix();

	// Draw Venus
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex4);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(venus));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(1, 50, 50);
	glPopMatrix();

	// Draw Earth
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(earth));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(2, 50, 50);
	glPopMatrix();

	// Draw Moon
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex2);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(moon));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(0.40, 50, 50);
	glPopMatrix();

	// Draw Mars
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex5);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(mars));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(1., 50, 50);
	glPopMatrix();

	// Draw Jupiter
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex6);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(jupiter));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(10, 50, 50);
	glPopMatrix();

	// Draw Saturn
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex7);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(saturn));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(8, 50, 50);
	glPopMatrix();

	// Draw Uranus
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex8);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(uranus));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(4, 50, 50);
	glPopMatrix();

	// Draw Neptune
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex9);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMultMatrixf(glm::value_ptr(neptune));
	glShadeModel(GL_SMOOTH);
	SetMaterial(1, 1, 1, 1.0);
	OsuSphere(4, 50, 50);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT2);

	// DRAWING ALL ORBITS:

	// Mercury Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang1 = 2. * M_PI / (float)(1000 - 1);
	float ang1 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(13.68 * cos(ang1), 0., 13.68 * sin(ang1));
		ang1 += dang1;
	}
	glEnd();
	glPopMatrix();

	// Venus Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang2 = 2. * M_PI / (float)(1000 - 1);
	float ang2 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(16.72 * cos(ang2), 0., 16.72 * sin(ang2));
		ang2 += dang2;
	}
	glEnd();
	glPopMatrix();

	// // Earth Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang3 = 2. * M_PI / (float)(1000 - 1);
	float ang3 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(19.29 * cos(ang3), 0., 19.29 * sin(ang3));
		ang3 += dang3;
	}
	glEnd();
	glPopMatrix();

	// // Mars Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang4 = 2. * M_PI / (float)(1000 - 1);
	float ang4 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(25 * cos(ang4), 0., 25 * sin(ang4));
		ang4 += dang4;
	}
	glEnd();
	glPopMatrix();

	// Jupiter Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang5 = 2. * M_PI / (float)(1000 - 1);
	float ang5 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(35 * cos(ang5), 0., 35 * sin(ang5));
		ang5 += dang5;
	}
	glEnd();
	glPopMatrix();

	// Saturn Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang6 = 2. * M_PI / (float)(1000 - 1);
	float ang6 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(60 * cos(ang6), 0., 60 * sin(ang6));
		ang6 += dang6;
	}
	glEnd();
	glPopMatrix();

	// Uranus Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang7 = 2. * M_PI / (float)(1000 - 1);
	float ang7 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(75 * cos(ang7), 0., 75 * sin(ang7));
		ang7 += dang7;
	}
	glEnd();
	glPopMatrix();

	// Neptune Orbit
	glPushMatrix();
	glColor3f(0., 1., 0.);
	float dang8 = 2. * M_PI / (float)(1000 - 1);
	float ang8 = 0.;
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		glVertex3f(100 * cos(ang8), 0., 100 * sin(ang8));
		ang8 += dang8;
	}
	glEnd();
	glPopMatrix();

#ifdef DEMO_Z_FIGHTING
	if (DepthFightingOn != 0)
	{
		glPushMatrix();
		glRotatef(90.f, 0.f, 1.f, 0.f);
		glCallList(BoxList);
		glPopMatrix();
	}
#endif

	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable(GL_DEPTH_TEST);
	glColor3f(0.f, 1.f, 1.f);
	// DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1.f, 1.f, 1.f);
	// DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}

void drawSun(int texture)
{
	if (texture == 2)
		Distorted = true;
	else
		Distorted = false;
	// glMatrixMode(GL_TEXTURE);

	OsuSphere(10, 50, 50);
}

void DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoColorMenu(int id)
{
	WhichColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDistortMenu(int id)
{
	ObjectTexture = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// main menu callback:

void DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoProjectMenu(int id)
{
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:

void DoRasterString(float x, float y, float z, char *s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c; // one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}

// use glut to display a string of characters using a stroke font:

void DoStrokeString(float x, float y, float z, float ht, char *s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c; // one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}

// return the number of seconds since the start of the program:

float ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}

// initialize the glui window:

void InitMenus()
{
	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(int));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int distortmenu = glutCreateMenu(DoDistortMenu);
	glutAddMenuEntry("No Texture", 0);
	glutAddMenuEntry("Textured", 1);
	glutAddMenuEntry("Textured and distortion", 2);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Axis Colors", colormenu);
	glutAddSubMenu("Distortion", distortmenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);
	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// initialize the glut and OpenGL libraries:
//	also setup callback functions

void InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	// glutPassiveMotionFunc( NULL );
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc(Animate);

	// init the glew package (a window must be open to do this):
	int width0, height0, width1, height1, width2, height2, width3, height3, width4, height4, width5, height5, width6, height6, width7, height7, width8, height8, width9, height9;
	unsigned char *TextureArray0 = BmpToTexture((char *)"suntex.bmp", &width0, &height0);
	unsigned char *TextureArray1 = BmpToTexture((char *)"earthtex.bmp", &width1, &height1);
	unsigned char *TextureArray2 = BmpToTexture((char *)"moontex.bmp", &width2, &height2);
	unsigned char *TextureArray3 = BmpToTexture((char *)"mercurytex.bmp", &width3, &height3);
	unsigned char *TextureArray4 = BmpToTexture((char *)"venustex.bmp", &width4, &height4);
	unsigned char *TextureArray5 = BmpToTexture((char *)"marstex.bmp", &width5, &height5);
	unsigned char *TextureArray6 = BmpToTexture((char *)"jupitertex.bmp", &width6, &height6);
	unsigned char *TextureArray7 = BmpToTexture((char *)"saturntex.bmp", &width7, &height7);
	unsigned char *TextureArray8 = BmpToTexture((char *)"uranustex.bmp", &width8, &height8);
	unsigned char *TextureArray9 = BmpToTexture((char *)"neptunetex.bmp", &width9, &height9);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &Tex0); // assign binding “handles”
	glGenTextures(1, &Tex1);
	glGenTextures(1, &Tex2);
	glGenTextures(1, &Tex3);
	glGenTextures(1, &Tex4);
	glGenTextures(1, &Tex5);
	glGenTextures(1, &Tex6);
	glGenTextures(1, &Tex7);
	glGenTextures(1, &Tex8);
	glGenTextures(1, &Tex9);

	glBindTexture(GL_TEXTURE_2D, Tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width0, height0, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray0);

	glBindTexture(GL_TEXTURE_2D, Tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray1);

	glBindTexture(GL_TEXTURE_2D, Tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray2);

	glBindTexture(GL_TEXTURE_2D, Tex3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray3);

	glBindTexture(GL_TEXTURE_2D, Tex4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width4, height4, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray4);

	glBindTexture(GL_TEXTURE_2D, Tex5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width5, height5, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray5);

	glBindTexture(GL_TEXTURE_2D, Tex6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width6, height6, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray6);

	glBindTexture(GL_TEXTURE_2D, Tex7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width7, height7, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray7);

	glBindTexture(GL_TEXTURE_2D, Tex8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width8, height8, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray8);

	glBindTexture(GL_TEXTURE_2D, Tex9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width9, height9, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureArray9);

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void InitLists()
{

	glutSetWindow(MainWindow);

	// create the object:

	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}

// the keyboard callback:

void Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT); // will not return here
		break;			  // happy compiler

	case '2':
		Light2On = !Light2On;
		break;

	case 'e':
	case 'E':
		WhichView = EARTHVIEW;
		break;

	case 'm':
	case 'M':
		WhichView = MOONVIEW;
		break;

	case 'k':
	case 'K':
		WhichView = OUT;
		break;

	case 'l':
	case 'L':
		WhichView = OUTSIDEVIEW;
		break;

	case 'f':
	case 'F':
		Freeze = !Freeze;
		if (Freeze)
			glutIdleFunc(NULL);
		else
			glutIdleFunc(Animate);
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// called when the mouse button transitions down or up:

void MouseButton(int button, int state, int x, int y)
{
	int b = 0; // LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;
		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;
		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;
		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b; // set the proper bit
	}
	else
	{
		ActiveButton &= ~b; // clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// called when the mouse moves while a button is down:

void MouseMotion(int x, int y)
{
	int dx = x - Xmouse; // change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x; // new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	Light2On = false;
}

// called when user resizes the window:

void Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// handle a change to the window's visibility:

void Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}

///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {0.f, 1.f, 0.f, 1.f};

static float xy[] = {-.5f, .5f, .5f, -.5f};

static int xorder[] = {1, 2, -3, 4};

static float yx[] = {0.f, 0.f, -.5f, .5f};

static float yy[] = {0.f, .6f, 1.f, 1.f};

static int yorder[] = {1, 2, 3, -2, 4};

static float zx[] = {1.f, 0.f, 1.f, 0.f, .25f, .75f};

static float zy[] = {.5f, .5f, -.5f, -.5f, 0.f, 0.f};

static int zorder[] = {1, 2, 3, 4, -5, 6};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();
}

// read a BMP file into a Texture:

#define VERBOSE false
#define BMP_MAGIC_NUMBER 0x4d42
#ifndef BI_RGB
#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#endif

// bmp file header:
struct bmfh
{
	short bfType; // BMP_MAGIC_NUMBER = "BM"
	int bfSize;	  // size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes; // # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		   // info header size, should be 40
	int biWidth;	   // image width
	int biHeight;	   // image height
	short biPlanes;	   // #color planes, should be 1
	short biBitCount;  // #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression; // BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed; // # colors in the palette
	int biClrImportant;
} InfoHeader;

// read a BMP file into a Texture:

unsigned char *
BmpToTexture(char *filename, int *width, int *height)
{
	FILE *fp;
#ifdef _WIN32
	errno_t err = fopen_s(&fp, filename, "rb");
	if (err != 0)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#else
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort(fp);

	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if (VERBOSE)
		fprintf(stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
				FileHeader.bfType, FileHeader.bfType & 0xff, (FileHeader.bfType >> 8) & 0xff);
	if (FileHeader.bfType != BMP_MAGIC_NUMBER)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}

	FileHeader.bfSize = ReadInt(fp);
	if (VERBOSE)
		fprintf(stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize);

	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);

	FileHeader.bfOffBytes = ReadInt(fp);

	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);

	InfoHeader.biBitCount = ReadShort(fp);
	if (VERBOSE)
		fprintf(stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount);

	InfoHeader.biCompression = ReadInt(fp);
	if (VERBOSE)
		fprintf(stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression);

	InfoHeader.biSizeImage = ReadInt(fp);
	if (VERBOSE)
		fprintf(stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage);

	InfoHeader.biXPixelsPerMeter = ReadInt(fp);
	InfoHeader.biYPixelsPerMeter = ReadInt(fp);

	InfoHeader.biClrUsed = ReadInt(fp);
	if (VERBOSE)
		fprintf(stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed);

	InfoHeader.biClrImportant = ReadInt(fp);

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char *texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\n");
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ((InfoHeader.biBitCount * InfoHeader.biWidth + 31) / 32);
	if (VERBOSE)
		fprintf(stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes);

	int myRowSizeInBytes = (InfoHeader.biBitCount * InfoHeader.biWidth + 7) / 8;
	if (VERBOSE)
		fprintf(stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes);

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if (VERBOSE)
		fprintf(stderr, "New NumExtra padding = %d\n", numExtra);

	// this function does not support compression:

	if (InfoHeader.biCompression != 0)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}

	// we can handle 24 bits of direct color:
	if (InfoHeader.biBitCount == 24)
	{
		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char *tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp); // b
				*(tp + 1) = fgetc(fp); // g
				*(tp + 0) = fgetc(fp); // r
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if (InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256)
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32 *colorTable = new struct rgba32[InfoHeader.biClrUsed];

		rewind(fp);
		fseek(fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET);
		for (int c = 0; c < InfoHeader.biClrUsed; c++)
		{
			colorTable[c].r = fgetc(fp);
			colorTable[c].g = fgetc(fp);
			colorTable[c].b = fgetc(fp);
			colorTable[c].a = fgetc(fp);
			if (VERBOSE)
				fprintf(stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
						c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a);
		}

		rewind(fp);
		fseek(fp, FileHeader.bfOffBytes, SEEK_SET);
		int t;
		unsigned char *tp;
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				int index = fgetc(fp);
				*(tp + 0) = colorTable[index].r; // r
				*(tp + 1) = colorTable[index].g; // g
				*(tp + 2) = colorTable[index].b; // b
			}

			for (int e = 0; e < numExtra; e++)
			{
				fgetc(fp);
			}
		}

		delete[] colorTable;
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

int ReadInt(FILE *fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	const unsigned char b2 = fgetc(fp);
	const unsigned char b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

short ReadShort(FILE *fp)
{
	const unsigned char b0 = fgetc(fp);
	const unsigned char b1 = fgetc(fp);
	return (b1 << 8) | b0;
}

// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)
		h -= 6.;
	while (h < 0.)
		h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.; // red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;

	case 1:
		r = q;
		g = v;
		b = p;
		break;

	case 2:
		r = p;
		g = v;
		b = t;
		break;

	case 3:
		r = p;
		g = q;
		b = v;
		break;

	case 4:
		r = t;
		g = p;
		b = v;
		break;

	case 5:
		r = v;
		g = p;
		b = q;
		break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

#include <stdio.h>
#include <math.h>

#ifndef POINT_H
#define POINT_H
struct point
{
	float x, y, z;	  // coordinates
	float nx, ny, nz; // surface normal
	float s, t;		  // texture coords
};

inline void
DrawPoint(struct point *p)
{
	glNormal3fv(&p->nx);
	glTexCoord2fv(&p->s);
	glVertex3fv(&p->x);
}
#endif

int SphNumLngs, SphNumLats;
struct point *SphPts;

inline struct point *
SphPtsPointer(int lat, int lng)
{
	if (lat < 0)
		lat += (SphNumLats - 1);
	if (lng < 0)
		lng += (SphNumLngs - 0);
	if (lat > SphNumLats - 1)
		lat -= (SphNumLats - 1);
	if (lng > SphNumLngs - 1)
		lng -= (SphNumLngs - 0);
	return &SphPts[SphNumLngs * lat + lng];
}

void OsuSphere(float radius, int slices, int stacks)
{
	// set the globals:

	SphNumLngs = slices;
	SphNumLats = stacks;
	if (SphNumLngs < 3)
		SphNumLngs = 3;
	if (SphNumLats < 3)
		SphNumLats = 3;

	// allocate the point data structure:

	SphPts = new struct point[SphNumLngs * SphNumLats];

	// fill the SphPts structure:

	for (int ilat = 0; ilat < SphNumLats; ilat++)
	{
		float lat = -M_PI / 2. + M_PI * (float)ilat / (float)(SphNumLats - 1); // ilat=0/lat=0. is the south pole
																			   // ilat=SphNumLats-1, lat=+M_PI/2. is the north pole
		float xz = cosf(lat);
		float y = sinf(lat);
		for (int ilng = 0; ilng < SphNumLngs; ilng++) // ilng=0, lng=-M_PI and
													  // ilng=SphNumLngs-1, lng=+M_PI are the same meridian
		{
			float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(SphNumLngs - 1);
			float x = xz * cosf(lng);
			float z = -xz * sinf(lng);
			struct point *p = SphPtsPointer(ilat, ilng);
			p->x = radius * x;
			p->y = radius * y;
			p->z = radius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = (lng + M_PI) / (2. * M_PI);
			p->t = (lat + M_PI / 2.) / M_PI;
			if (!Distorted)
			{
				p->s = (lng + M_PI) / (2. * M_PI);
				p->t = (lat + M_PI / 2.) / M_PI;
			}
			else
			{
				p->s = ((lng + M_PI) / (2. * M_PI)) * (2 + sin(3 * Time));
				p->t = (lat + M_PI / 2.) / M_PI;
			}
		}
	}

	struct point top, bot; // top, bottom points

	top.x = 0.;
	top.y = radius;
	top.z = 0.;
	top.nx = 0.;
	top.ny = 1.;
	top.nz = 0.;
	top.s = 0.;
	top.t = 1.;

	bot.x = 0.;
	bot.y = -radius;
	bot.z = 0.;
	bot.nx = 0.;
	bot.ny = -1.;
	bot.nz = 0.;
	bot.s = 0.;
	bot.t = 0.;

	// connect the north pole to the latitude SphNumLats-2:

	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = 0; ilng < SphNumLngs; ilng++)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(SphNumLngs - 1);
		top.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&top);
		struct point *p = SphPtsPointer(SphNumLats - 2, ilng); // ilat=SphNumLats-1 is the north pole
		DrawPoint(p);
	}
	glEnd();

	// connect the south pole to the latitude 1:

	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = SphNumLngs - 1; ilng >= 0; ilng--)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(SphNumLngs - 1);
		bot.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&bot);
		struct point *p = SphPtsPointer(1, ilng); // ilat=0 is the south pole
		DrawPoint(p);
	}
	glEnd();

	// connect the horizontal strips:

	for (int ilat = 2; ilat < SphNumLats - 1; ilat++)
	{
		struct point *p;
		glBegin(GL_TRIANGLE_STRIP);
		for (int ilng = 0; ilng < SphNumLngs; ilng++)
		{
			p = SphPtsPointer(ilat, ilng);
			DrawPoint(p);
			p = SphPtsPointer(ilat - 1, ilng);
			DrawPoint(p);
		}
		glEnd();
	}

	// clean-up:

	delete[] SphPts;
	SphPts = NULL;
}

void SetPointLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.5);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

float *
Array3(float a, float b, float c)
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

float *
MulArray3(float factor, float array0[3])
{
	static float array[4];
	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

void SetMaterial(float r, float g, float b, float shininess)
{
	glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, White));
	glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
	glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
	glMaterialf(GL_BACK, GL_SHININESS, 2.f);
	glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}