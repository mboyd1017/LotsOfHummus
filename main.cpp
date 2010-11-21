////////////////////////////////////////////////////////////////////////
//
//   Harvard Computer Science
//	  CS 175: Introduction to Computer Graphics
//   Professor: Hanspeter Pfister
//   TFs: Moritz Baecher, Yuanchen Zhu, Kalyan Sunkavalli, Kevin Dale
//
//   File: main.cpp
//   Name: <insert your name here>
//   Date: <insert date>
//   Desc: <add description here>
//   
////////////////////////////////////////////////////////////////////////

// I N C L U D E S /////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef __MAC__
#	include <GLUT/glut.h>
#else
#  define FREEGLUT_STATIC
#  include <GL/glut.h>
#endif

#include "shaders.h"
#include "trackball.c"						// trackball quaternion routines
#include "Vector.h"							// basic vector class
#include "Matrix.h"							// basic Matrix class

using namespace std;

// G L O B A L S ///////////////////////////////////////////////////////

// Screen dimensions
#define	SCREEN_WIDTH	800
#define SCREEN_HEIGHT	400
#define SCREEN_FOV		30
static int sw, sh; 									// current screen dimensions

//Rotation angles
#define SUN_SPEED	10
#define EARTH_SPEED 10
#define EARTH_ORB_SPEED 30
#define MOON_SPEED 10
#define MOON_ORB_SPEED 60
#define TIMER_INC 100

static float SPEED_CONST = 0.2;
static float sunTheta = 0;
static float moonTheta = 0;
static float moonOrbTheta = 0;
static float earthTheta = 0;
static float earthOrbTheta = 0;
static bool paused = false;
static bool cones = false;

#define KEY_SPEED 0.1
static float FOVDeg = 45.0;
static float cameraPos[3] = {0.0f, 0.0f, 5.0f};
static float scenePos[3] = {0.0f, 0.0f, 0.0f};
static float cameraRotation[2] = {0.0f, 0.0f};
#define CAMERA_ROT_DEG 20

//quaternions to keep track of the viewing direction
static float quatCurr[4];							// current view quaternion
static float quatLast[4];							// last view quaternion
static int beginx, beginy;							// variables useful for quaternion rotations and mouse interface
static bool moving;									// the mouse is moving

// FUNCTION DEFINITIONS
void changeFOV(float);
void translateCamera(float x, float y, float z);
void rotateCamera(float x, float y);

// H E L P E R    F U N C T I O N S ////////////////////////////////////

// Checks if there's an error inside OpenGL and if yes,
// print the error the throw an exception.
void checkGLErrors() {
	const GLenum errCode = glGetError();      // check for errors
	if (errCode != GL_NO_ERROR) {
		std::cerr << "Error: " << gluErrorString(errCode) << std::endl;
		throw runtime_error((const char*)gluErrorString(errCode));
	}
}

// R E N D E R S T A T E S /////////////////////////////////////////////

struct GlslProgram {
	// Handle to the OpenGL shading program object
	GLuint program;   

	// Handles to uniform variables for the shading program
	GLuint fvLightPositionHandle; 
	GLuint fvEyePositionHandle;
	GLuint fvAmbientHandle;
	GLuint fvSpecularHandle;
	GLuint fvDiffuseHandle;
	GLuint fSpecularPowerHandle;
	GLuint fvBaseColorHandle;
};

// Shader related parameters
static const int NUM_SHADERS = 2;
static char* glslSources[NUM_SHADERS][2] = {{"phong.vert", "phong.frag"}, {"phongOren.vert", "phongOren.frag"}};

// Vector of structs storing OpenGL program handles and uniform variable handles
static vector<GlslProgram> glslPrograms;

// Which shader to use, in the range [0, NUM_SHADES)
static int activeShader = 0; 

// Values of uniform parameter. Note that to pass them to the
// shader program, you need to explicit call glUniform*
#define VIEW_DISTANCE 5.0f
static float fvLightPosition[] = {0.f, 0.f, 10.f};
static float fvEyePosition[] = {0.f, 0.f, VIEW_DISTANCE};
static float fvDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
static float fvSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
static float fvAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
static float fSpecularPower[] = {25};
static float fvBaseColorSun[] = {0.7f, 0.7f, 0.0f, 1.0f};
static float fvBaseColorEarth[] = {0.0f, 0.3f, 0.7f, 1.0f};
static float fvBaseColorMoon[] = {0.7f, 0.7f, 0.7f, 1.0f};

static int tessLevel = 50; // How finely the sphere is tesselated

void initPhongProgram(GlslProgram& program, const char* vsFile, const char *fsFile)
{
	GLuint p = compileShaders(vsFile, fsFile);
	program.program = p;

	// Grab handle to the compiled GLSL program and store them for later use.
	program.fvLightPositionHandle = glGetUniformLocation(p, "fvLightPosition");
	program.fvEyePositionHandle = glGetUniformLocation(p, "fvEyePosition");
	program.fvAmbientHandle = glGetUniformLocation(p, "fvAmbient");
	program.fvSpecularHandle = glGetUniformLocation(p, "fvSpecular");
	program.fvDiffuseHandle = glGetUniformLocation(p, "fvDiffuse");
	program.fSpecularPowerHandle = glGetUniformLocation(p, "fSpecularPower");
	program.fvBaseColorHandle = glGetUniformLocation(p, "fvBaseColor");
}

void initGLSL()
{
	glslPrograms.resize(NUM_SHADERS);
	for (int i = 0; i < NUM_SHADERS; ++i)
		initPhongProgram(glslPrograms[i], glslSources[i][0], glslSources[i][1]);
}

void printDebugMessages()
{
	cout << "FOVDeg " << FOVDeg << endl;
}

void render()
{
	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the right projection matrix for the camera here

	// compute the rotation matrix from the quaternion
	GLfloat matRotation[4][4];
	build_rotmatrix(matRotation, quatCurr);

	// set the modelview matrix -- use the rotation matrix you get above to set this properly
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt( cameraPos[0], cameraPos[1], cameraPos[2], // camera center
			scenePos[0], scenePos[1], scenePos[2], // scene center
			  0.0f, 1.0f, 0.0f); // camera up vector
	// Quaternion/mouse camera control
	glMultMatrixf(*matRotation);
	
	// testing camera rotation
	glRotatef(cameraRotation[0], 0.f, 1.f, 0.f);
	glRotatef(cameraRotation[1], 1.0f, 0.f, 0.f);

	// sets the shader program
	const GlslProgram& p = glslPrograms[activeShader];
	float fvBaseColor[4];

	glUseProgram(p.program);
	glUniform3fv(p.fvLightPositionHandle, 1, fvLightPosition);
	glUniform3fv(p.fvEyePositionHandle, 1, fvEyePosition);
	glUniform4fv(p.fvAmbientHandle, 1, fvAmbient);
	glUniform4fv(p.fvSpecularHandle, 1, fvSpecular);
	glUniform4fv(p.fvDiffuseHandle, 1, fvDiffuse);
	glUniform1fv(p.fSpecularPowerHandle, 1, fSpecularPower);

	// transform the scene objects to animate them

	// draw the sun
	fvBaseColor[0] = fvBaseColorSun[0];
	fvBaseColor[1] = fvBaseColorSun[1];
	fvBaseColor[2] = fvBaseColorSun[2];
	fvBaseColor[3] = fvBaseColorSun[3];
	glUniform4fv(p.fvBaseColorHandle, 1, fvBaseColor);
	
	// Sun rotation independent of everything else;
	// in Matrix block
	glPushMatrix();
	// Sun Rotation
	glRotatef(sunTheta, 0.5, 1.0, 0.2);
	if (cones)
	{
		glutSolidCone(0.4, 0.4, tessLevel * 2, tessLevel);
	}
	else
	{
		glutSolidSphere(0.4, tessLevel*2, tessLevel);
	}
	glPopMatrix();

	// Earth Orbits
	// glRotatef(earthOrbTheta, 0.0, -1.0, 0.0);
	//glTranslatef(2.0, 0.0, 0.0);

	// ...ELLIPTICALLY!
	glTranslatef(2.0f*sin(earthOrbTheta), 0.0, 1.0f*cos(earthOrbTheta));

	// draw the earth
	fvBaseColor[0] = fvBaseColorEarth[0];
	fvBaseColor[1] = fvBaseColorEarth[1];
	fvBaseColor[2] = fvBaseColorEarth[2];
	fvBaseColor[3] = fvBaseColorEarth[3];
	glUniform4fv(p.fvBaseColorHandle, 1, fvBaseColor);

	// Earth Rotates
	glPushMatrix();
	glRotatef(earthTheta, 0.0, 1.0, 1.0);

	if (cones)
	{
		glutSolidCone(0.2, 0.2, tessLevel * 2, tessLevel);
	}
	else
	{
		glutSolidSphere(0.2, tessLevel*2, tessLevel);
	}
	glPopMatrix();
	
	// Moon orbits earth

	//glRotatef(moonOrbTheta, 0.0, 0.0, 1.0);
	//glTranslatef(0.0, 0.5, 0.0);

	// Eliptical orbits
	glTranslatef(0.5f*cos(moonOrbTheta), 2.0f*sin(moonOrbTheta), 0.0);

	// draw the moon
	fvBaseColor[0] = fvBaseColorMoon[0];
	fvBaseColor[1] = fvBaseColorMoon[1];
	fvBaseColor[2] = fvBaseColorMoon[2];
	fvBaseColor[3] = fvBaseColorMoon[3];
	glUniform4fv(p.fvBaseColorHandle, 1, fvBaseColor);
	
	glPushMatrix();
	// Moon Rotates
	glRotatef(moonTheta, -1.0, 0.0, 0.0);
	if (cones)
	{
		glutSolidCone(0.1, 0.1, tessLevel * 2, tessLevel);
	}
	else
	{
		glutSolidSphere(0.1, tessLevel*2, tessLevel);
	}
	glPopMatrix();
	printDebugMessages();
	checkGLErrors();
}

// C A L L B A C K S ///////////////////////////////////////////////////

// _____________________________________________________
//|														|
//|	 reshape											|
//|_____________________________________________________|
///
///  Whenever a window is resized, a "resize" event is
///  generated and GLUT is told to call this reshape
///  callback function to handle it appropriately.

void reshape(int w, int h)
{
	// update the screen dimensions
	sw = w;
	sh = h;

	cout << "Reshaping the window." << endl;

	// reset the projection matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( FOVDeg, // field of view angle in degrees
		(float) sw / (float) sh, // aspect ratio 
		100.0, // distance from the viewer to the near plane
		1.0 // distance from the viewer to the far plane
		);

	// reset the viewport
	glViewport(0, 0, sw, sh);
}


// _____________________________________________________
//|														|
//|	 display											|
//|_____________________________________________________|
///
///  Whenever OpenGL requires a screen refresh
///  it will call display() to draw the scene.
///  We specify that this is the correct function
///  to call with the glutDisplayFunc() function
///  during initialization

void display(void) {
	// perform OpenGL rendering
	render();

	// swap buffers
	glutSwapBuffers();
}

// _____________________________________________________
//|														|
//|	 keyboard											|
//|_____________________________________________________|
///
///  Whenever a key on the keyboard is pressed, 
///  a "keyboard" event is generated and GLUT is told 
///  to call this keyboard callback function to handle it
///  appropriately.

void keyboard(unsigned char key, int x, int y) 
{
	// handle calls to change the field of view and zoom in and zoom out here
	switch (key) 
	{
		case 't':
			if (++activeShader == NUM_SHADERS)
				activeShader = 0;
			break;
		case 'r':
			// reset the quaternion to its original state
			trackball(quatCurr, 0.0, 0.0, 0.0, 0.0);
			break;
		case 'z':
			changeFOV(2.0);
			break;
		case 'x':
			changeFOV(-2.0);
			break;
		case 'n':
			translateCamera(0.0f, 0.0f, 10.0f);
			break;
		case 'm':
			translateCamera(0.0f, 0.0f, -10.0f);
			break;
		// WASD Rotate Camera
		case 'w':
			rotateCamera(0.f, (CAMERA_ROT_DEG));
			break;
		case 'a':
			rotateCamera(-(CAMERA_ROT_DEG), 0.0f);
			break;
		case 's':
			rotateCamera(0.f, -(CAMERA_ROT_DEG));
			break;
		case 'd':
			rotateCamera((CAMERA_ROT_DEG), 0.0f);
			break;
		case 'p':
			paused = !paused;
			break;
		// Toggle Cones/Spheres
		case 'c':
			cones = !cones;
			break;
		case 27:
			exit(0);
			break;
		default:
			;
	}
	glutPostRedisplay();
}

// _____________________________________________________
//|														|
//|	 special keyboard									|
//|_____________________________________________________|
///
///  Whenever a special key on the keyboard is pressed, 
///  a "special keyboard" event is generated and GLUT is told 
///  to call this special keyboard callback function to handle it
///  appropriately. Special keys are define dby the GLUT_KEY_*
///  constants.

void specialKeyboard(int key, int x, int y)
{
	//handle calls to translate the camera here
	switch (key) {
	case GLUT_KEY_LEFT:
		translateCamera(1.0f, 0.0f, 0.0f);
		break;
	case GLUT_KEY_RIGHT:
		translateCamera(-1.0f, 0.0f, 0.0f);
		break;
	case GLUT_KEY_UP:
		translateCamera(0.0f, -1.0f, 0.0f);
		break;
	case GLUT_KEY_DOWN:
		translateCamera(0.0f, 1.0f, 0.0f);
		break;
	default:
		break;
	}
}


// _____________________________________________________
//|														|
//|	 mouse											    |
//|_____________________________________________________|
///
///  Whenever a mouse button is clicked, a "mouse" event 
///  is generated and GLUT call this mouse callback to 
///  handle it appropriately.

void mouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		cout << "Mouse left button down." << endl;
		moving = 1;
		beginx = x;
		beginy = y;
	}
	else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		cout << "Mouse left button up." << endl;
		moving = 0;
	}
	glutPostRedisplay();
}


// _____________________________________________________
//|														|
//|	 motion											    |
//|_____________________________________________________|
///
///  Moving the mouse generates a "motion" event that can be 
///  handled in this motion callback function. Using this
///  in combination with the "mouse" callback will allow us
///  to interact with the scene using the mouse.

void motion(int x, int y)
{
	if(moving)
	{
		cout << "Rotating view." << endl;
		trackball(quatLast, (2.0 * beginx - sw) / sw, (sh - 2.0 * beginy) / sh,	(2.0 * x - sw) / sw, (sh - 2.0 * y) / sh);
		beginx = x;
		beginy = y;
		add_quats(quatLast, quatCurr, quatCurr);
	}
	glutPostRedisplay();
}

// I N I T S ///////////////////////////////////////////////////////////

void initsGLUTState( )
{
	// use RGBA pixel channels, double buffering, and depth buffering
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// choose initial window size 
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	// create a window with title "CS 175 HW1"
	glutCreateWindow( "CS 175 HW2" );

	// register the callback functions
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specialKeyboard);
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
}

void initsGLState()
{
	// set the clear color to black
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glClearDepth(0.0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);

	// initialize the screen dimensions and quaternion
	sw = SCREEN_WIDTH;
	sh = SCREEN_HEIGHT;

	moving = 0;
	trackball(quatCurr, 0.0, 0.0, 0.0, 0.0);

	// set up the projection matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective(45, // field of view angle in degrees
		(float) sw / (float) sh, // aspect ratio 
		100.0, // distance from the viewer to the near plane
		1.0 // distance from the viewer to the far plane
		);

	// set up the viewport
	glViewport(0, 0, sw, sh);

	initGLSL();

	checkGLErrors();
}

void sunTimerCallback(int arg) {
if (!paused)
{
	sunTheta += SPEED_CONST*SUN_SPEED;
		glutPostRedisplay();
}
	glutTimerFunc(TIMER_INC, sunTimerCallback, 0);
}

void earthTimerCallback(int arg) {
if (!paused)
{
	earthTheta += SPEED_CONST*EARTH_SPEED;
	earthOrbTheta += SPEED_CONST*EARTH_ORB_SPEED;
}
	glutTimerFunc(TIMER_INC, earthTimerCallback, 0);
	glutPostRedisplay();
}

void moonTimerCallback(int arg) {
	if (!paused)
	{
		moonTheta += SPEED_CONST*MOON_SPEED;
		moonOrbTheta += SPEED_CONST*MOON_ORB_SPEED;
		glutPostRedisplay();
	}
		glutTimerFunc(TIMER_INC, moonTimerCallback, 0);	
}

void changeFOV(float deg)
{
	FOVDeg = (FOVDeg + deg);
}

void translateCamera(float x, float y, float z)
{
	scenePos[0] += (KEY_SPEED*x);
	scenePos[1] += (KEY_SPEED*y);
	cameraPos[2] += (KEY_SPEED*z);
}

void rotateCamera(float x, float y)
{
	cameraRotation[0] += x;
	cameraRotation[1] += y;
}

int main(int argc, char **argv) 
{
	try {
		// initialize GLUT based on command-line arguments
		glutInit(&argc, argv); 

		// initialize GLUT states
		initsGLUTState();

		// initialize GLEW
		glewInit();

		// check if at least OpenGL version 2.0 is supported
		if (glewIsSupported("GL_VERSION_2_0"))
			printf("Ready for OpenGL 2.0\n");
		else {
			throw runtime_error("OpenGL 2.0 not supported\n");
		}

		// initialize OpenGL states
		initsGLState();

		checkGLErrors();

		// Initialize callbacks:
		glutTimerFunc(TIMER_INC, sunTimerCallback, 0);
		glutTimerFunc(TIMER_INC, earthTimerCallback, 0);
		glutTimerFunc(TIMER_INC, moonTimerCallback, 0);

		// enter the GLUT main (event) loop
		glutMainLoop();
		return 0;
	}
	catch (exception& e) {
		cout << "Exception caught: " << e.what() << endl;
		return -1;
	}
}

