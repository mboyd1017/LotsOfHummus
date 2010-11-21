////////////////////////////////////////////////////////////////////////
//
//   Harvard Computer Science
//	  CS 175: Introduction to Computer Graphics
//   Professor: Hanspeter Pfister
//   TFs: Moritz Baecher, Yuanchen Zhu, Kalyan Sunkavalli, Kevin Dale
//
//   File: main.cpp
//   Name: Madelaine Boyd
//   Date: 10/26/10
//   Desc: HW 4
//   
////////////////////////////////////////////////////////////////////////

// I N C L U D E S /////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
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

#include "GLTools.h"

#include "shaders.h"

#include "Camera.h"

#include "trackball.c"


using namespace std;

// CONSTANTS
// ----------------------------------------------------------------------------------------------------

// initial screen dimensions
static const int INIT_SCREEN_WIDTH = 800;
static const int INIT_SCREEN_HEIGHT = 400;
static int sw, sh; 			

// projection parameters
static const float FOV = 45.0f;
static const float FAR_PLANE = 0.01f;
static const float NEAR_PLANE = 100.0f;

// ----------------------------------------------------------------------------------------------------

// GLOBALS
// ----------------------------------------------------------------------------------------------------

// camera parameters
Camera cam;
static float camInitCenter[] = { 0.0f, 2.0f, 0.0f };	// init camera center / eye vector
static float camInitUp[] = { 0.0f, 0.0f, 1.0f };		// init up vector 
static float camInitLook[] ={ 0.0f, 0.0f, 0.0f };		// init scene center
static float camRoll = 0.0f;
static float camPitch = 0.0f;
static float camYaw = 0.0f;

// How finely the mouse moves the camera
static float camMouseFine = 0.1f;

// projection, view and model matrices
static float fmProj[16];
static float fmView[16];
static float fmModel[16];

// shader parameters 
static float fvLightWorldCoords[] = {-5.0f, 2.0f, 0.0f};
static float fvEye[] = { 0.0f, 1.0f, 0.0f};
static float fvAmbient[] = {0.8f, 0.8f, 0.8f, 0.5f};
static float fvDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
static float fvSpecular[] = {0.1f, 0.1f, 0.1f, 1.0f};
static float fSpecularPower[] = {25.0};
static float fvBaseColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

static float sunlightD[4] = {0.0, 1.0, 0.0, 0.0};
static float light_ambient2[4]  = {0.2f, 0.2f, 2.0, 1.0};
static float light_diffuse2[4]  = {0.8f, 0.8f, 1.0, 1.0};
static float light_specular2[4] = {0.0, 0.0, 1.0, 1.0};

//Model3D landspeeder;
#define LANDSPEEDER_FILE = ".//landspeeder//LandSpeeder.obj"

// flags
bool drawClouds = false;
enum DrawMode { DRAW_PHONG, DRAW_COLOR_MAP, DRAW_MARS, DRAW_BUMP_MAP } activeDrawMode = DRAW_MARS;
bool drawSkyBox = true;

// orbit angles
bool animate = true;
static float earthRotAngle = 0.0f;
static float sunRotAngle = 0.0f;
static float moonRotAngle = 0.0f;

//quaternions to keep track of the viewing direction
static float quatCurr[4];							// current view quaternion
static float quatLast[4];							// last view quaternion
static int beginx, beginy;							// variables useful for quaternion rotations and mouse interface
static bool moving;									// the mouse is moving

GLuint quadsToRender = 100000;

unsigned int throttle = 0;
static int MAX_THROTTLE = 10;


// ----------------------------------------------------------------------------------------------------

// HELPER FUNCTIONS
// ----------------------------------------------------------------------------------------------------

// Checks if there's an error inside OpenGL and if yes,
// print the error the throw an exception.
void checkGLErrors() {
	const GLenum errCode = glGetError();      // check for errors
	if (errCode != GL_NO_ERROR) {
		std::cerr << "Error: " << gluErrorString(errCode) << std::endl;
		throw runtime_error((const char*)gluErrorString(errCode));
	}
}

// ----------------------------------------------------------------------------------------------------

// SHADER HANDLING
// ----------------------------------------------------------------------------------------------------

// number of shaders
static const int NUM_SHADERS = 5;

// shader program handles
GLuint shaderProgramHandles[ NUM_SHADERS ];

/*
// constants for shaders
enum Shaders { SHADER_PHONG, SHADER_COLOR_MAP, SHADER_BUMP_MAP, SHADER_CLOUD_MAP, SHADER_SKYBOX };

// file names for shaders
static const char *shaderProgramFileNames[NUM_SHADERS][2] = {  {"phong.vert", "phong.frag"},
                                                               {"colormap.vert", "colormap.frag"}, 
                                                               {"bumpmap.vert", "bumpmap.frag"}, 
                                                               {"cloudmap.vert", "cloudmap.frag"}, 
                                                               {"skybox.vert", "skybox.frag"}};
															   */
// constants for shaders
enum Shaders { SHADER_PHONG, SHADER_COLOR_MAP, SHADER_MARS, SHADER_SKYBOX, SHADER_CLOUD_MAP };

// file names for shaders
static const char *shaderProgramFileNames[NUM_SHADERS][2] = {  {"phong.vert", "phong.frag"},
                                                               {"colormap.vert", "colormap.frag"}, 
                                                               {"colormap.vert", "colormap.frag"}, 
                                                               {"skybox.vert", "skybox.frag"}, 
															   {"cloudmap.vert", "cloudmap.frag"} };

// ----------------------------------------------------------------------------------------------------

// TEXTURE MAPPING
// ----------------------------------------------------------------------------------------------------
// number of textures
static const int NUM_TEXTURES = 10;

// texture handles
GLuint textureHandles[ NUM_TEXTURES ];

// constants for textures
enum Textures {   TEXTURE_EARTH_COLOR_MAP, TEXTURE_EARTH_NORMAL_MAP, 
                  TEXTURE_EARTH_CLOUD_MAP, TEXTURE_EARTH_CLOUD_TRANS_MAP, 
                  TEXTURE_MOON_COLOR_MAP, TEXTURE_MOON_NORMAL_MAP, 
                  TEXTURE_SUN_COLOR_MAP, 
				  TEXTURE_MARS_COLOR_MAP, TEXTURE_MARS_HEIGHT_MAP, TEXTURE_MARS_NORMAL_MAP };

// file names for textures
const char *textureFileNames[ NUM_TEXTURES ] = {   ".//planets//earth//earth_colormap_4k.tga", 
                                                   ".//planets//earth//earth_normalmap_4k.tga", 
                                                   ".//planets//earth//earth_cloudmap.tga", 
                                                   ".//planets//earth//earth_cloudtransmap.tga",
                                                   ".//planets//moon//moon_colormap_4k.tga", 
                                                   ".//planets//moon//moon_normalmap_4k.tga", 
                                                   ".//planets//sun//sun_colormap.tga",
												   ".//mars//mars_1k_color.tga",
												   ".//mars//mars_1k_topo.tga",
												   ".//mars//mars_1k_normal.tga"
                                                   };

// ----------
//
//	Storing texture height data
// ----------------
GLfloat *marsHeightMap = NULL;
GLfloat *marsHeightColors = NULL;
GLfloat *marsHeightNormals = NULL;
GLubyte *marsHeightIndices = NULL;
GLfloat *marsHeightVertices = NULL;


/* Spare Function declarations */
void loadMarsHeightMap(GLbyte *, GLint , GLint );
void loadMarsColorMap(GLbyte *, GLint , GLint );
void loadMarsHeightNormals(GLbyte *, GLint , GLint );
void loadMarsHeightIndices(GLint , GLint );
void debugArray(GLfloat*, int, int);

void loadTextures( )
{
   checkGLErrors();
   // get number of texture units
   GLint numTextureUnits;
   glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &numTextureUnits );
   cout << "There are " << numTextureUnits << " texture units available." << endl; 

   // create texture handles for texture objects
   glGenTextures( NUM_TEXTURES, textureHandles );
   for( int textureIndex = 0; textureIndex < NUM_TEXTURES; textureIndex++ )
   {
      checkGLErrors();
      // bind texture object to texture handle
      glBindTexture( GL_TEXTURE_2D, textureHandles[ textureIndex ] );
      
      // load texture object from file into CPU memory
      GLbyte *pTextureData;
      GLint textureWidth, textureHeight, textureComponents;
      GLenum textureFormat;
      
      // use a GLTools library function to read the pixel data from the .tga files
      pTextureData = gltReadTGABits( textureFileNames[ textureIndex ], &textureWidth, &textureHeight, &textureComponents, &textureFormat );
      assert( pTextureData != NULL );
      cout << "Texture " << textureFileNames[ textureIndex ] << " loaded. W: " << textureWidth << " H: " << textureHeight << " C: " << textureComponents << " F: " << textureFormat << endl;
	 

	  /* Store the Mars texture data - we'll need it to render the mesh */
	  if(textureIndex == TEXTURE_MARS_HEIGHT_MAP)
	  {	
		  loadMarsHeightIndices(textureWidth, textureHeight);
		  loadMarsHeightMap(pTextureData, textureWidth, textureHeight);
	  }
	  /* Store mars normal information as RGBRGBRGB in marsHeightNormals */
	  else if (textureIndex == TEXTURE_MARS_NORMAL_MAP)
	  {
		  loadMarsHeightNormals(pTextureData, textureWidth, textureHeight);
	  }
	  else if (textureIndex == TEXTURE_MARS_COLOR_MAP)
	  {
		  //loadMarsColorMap(pTextureData, textureWidth, textureHeight);
	  }

      // set minification filter
      GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
      
      // set magnification filter
      GLenum magFilter = GL_LINEAR;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

      // set texture wrap modes
      GLenum wrapMode = GL_REPEAT;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

      // load texture object into GPU memory
      glTexImage2D( GL_TEXTURE_2D, 0, textureComponents, textureWidth, textureHeight, 0, textureFormat, GL_UNSIGNED_BYTE, pTextureData );

      // generate mipmaps
      glGenerateMipmap( GL_TEXTURE_2D );

      // free memory for texture object in CPU memory
      free( pTextureData );
      checkGLErrors();
   }
   checkGLErrors();
}

GLfloat scaleTexel(GLbyte pTextureData) {
	return (GLfloat) (pTextureData + 128) / 256.0f;
}

// Scales from 0-1 to -1-1
GLfloat scaleNormal(GLfloat value) {
	return 2.0*(value - 0.5);
}

inline float clamp(float x, float a, float b)
{
    return x < a ? a : (x > b ? b : x);
}

void loadMarsHeightIndices(GLint textureWidth, GLint textureHeight)
{
	int size8 = 8*textureWidth*textureHeight;
		marsHeightIndices = new GLubyte[size8];

		for (int i=0; i < textureHeight; i++)
		{
			for (int j=0; j < textureWidth; j++)
			{
				int idx = 8*(i*textureHeight + j);
				marsHeightIndices[idx] = (GLubyte)j;
				marsHeightIndices[idx + 1] = (GLubyte)i;
				marsHeightIndices[idx + 2] = (GLubyte)j;
				marsHeightIndices[idx + 3] = (GLubyte)i+1;
				marsHeightIndices[idx + 4] = (GLubyte)j+1;
				marsHeightIndices[idx + 5] = (GLubyte)i+1;
				marsHeightIndices[idx + 6] = (GLubyte)j+1;
				marsHeightIndices[idx + 7] = (GLubyte)i;
			}
		}
}

void loadMarsHeightNormals(GLbyte *pTextureData, GLint textureWidth, GLint textureHeight)
{
		unsigned int count = textureWidth*textureHeight;
		  unsigned int row = 3*textureHeight;
		  int depth = 12;
		  marsHeightNormals = new GLfloat[count*depth];

		  for (unsigned int ii=0; ii < count; ii++)
		  {
			  unsigned int i=3*ii;
			  
			  marsHeightNormals[i] = scaleNormal(scaleTexel(pTextureData[ii]));
			  marsHeightNormals[i+1] = scaleNormal(scaleTexel(pTextureData[ii+1]));
			  marsHeightNormals[i+2] = scaleNormal(scaleTexel(pTextureData[ii+2]));

			  marsHeightNormals[i+3] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight]));
			  marsHeightNormals[i+4] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight+1]));
			  marsHeightNormals[i+5] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight+2]));

			  marsHeightNormals[i+6] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight+3]));
			  marsHeightNormals[i+7] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight+4]));
			  marsHeightNormals[i+8] = scaleNormal(scaleTexel(pTextureData[ii+textureHeight+5]));

			  marsHeightNormals[i+9] = scaleNormal(scaleTexel(pTextureData[ii+3]));
			  marsHeightNormals[i+10] = scaleNormal(scaleTexel(pTextureData[ii+4]));
			  marsHeightNormals[i+11] = scaleNormal(scaleTexel(pTextureData[ii+5]));

		  }
		  debugArray(marsHeightNormals, 3, 100);
}

void loadMarsColorMap(GLbyte *pTextureData, GLint textureWidth, GLint textureHeight)
{
	GLint size12 = 12*textureWidth*textureHeight;
	marsHeightColors = new GLfloat[size12];
		for (int i=0; i < textureHeight; i++ )
		{
			for (int j=0; j < textureWidth; j++)
			{
				GLint curIdx = 12*(i*textureHeight + j);
				// TEST
				
				// P0
				marsHeightColors[curIdx] = scaleTexel(pTextureData[i]);
				marsHeightColors[curIdx + 1] = scaleTexel(pTextureData[i + 1]);
				marsHeightColors[curIdx + 2] = scaleTexel(pTextureData[i + 2]);

				// P0 + dX
				marsHeightColors[curIdx + 3] = scaleTexel(pTextureData[i + 3]);
				marsHeightColors[curIdx + 4] = scaleTexel(pTextureData[i + 4]);
				marsHeightColors[curIdx + 5] = scaleTexel(pTextureData[i + 5]);

				//P0 + dX + dZ
				marsHeightColors[curIdx + 6] = scaleTexel(pTextureData[i + textureHeight + 3]);
				marsHeightColors[curIdx + 7] = scaleTexel(pTextureData[i + textureHeight + 4]);
				marsHeightColors[curIdx + 8] = scaleTexel(pTextureData[i + textureHeight + 5]);

				// P0 + dZ
				marsHeightColors[curIdx + 9] = scaleTexel(pTextureData[i + textureHeight + 0]);
				marsHeightColors[curIdx + 10] = scaleTexel(pTextureData[i + textureHeight + 1]);
				marsHeightColors[curIdx + 11] = scaleTexel(pTextureData[i + textureHeight + 2]);
				

			}
		}
		cout << "MARS HEIGHT COLORS" << endl;
		debugArray(marsHeightColors, 3, 200);
}

/** Populates the marsHeightColors, marsHeightMap, and marsHeightIndices arrays */
void loadMarsHeightMap(GLbyte *pTextureData, GLint textureWidth, GLint textureHeight) 
{
		GLint size = textureWidth*textureHeight;
		GLint size3 = 3*textureWidth*textureHeight;
		GLint size8 = 8*size;
		GLint size12 = 12*size;
		//marsHeightMap = new GLfloat[size3];
		

		marsHeightVertices = new GLfloat[size12];
		
		//GLfloat xStart = 0.0 - textureWidth / 2;
		//GLfloat zStart = 0.0 - textureHeight / 2;

		GLfloat xStart = 0.0;
		GLfloat zStart = 0.0;

		GLfloat scaleFac = 0.02;
		for (int i=0; i<textureHeight - 10; i++)
		{
			for (int j=0; j<textureWidth - 10; j++)
			{
				GLuint heightIdx = (i*textureHeight + j);
				GLuint arrayIdx = 12*(i*textureHeight + j);
				GLfloat curX = scaleFac*(xStart + i);
				GLfloat curZ = scaleFac*(zStart + j);
				GLfloat dX = scaleFac*1;
				GLfloat dZ = scaleFac*1;
				GLfloat heightScale = 0.2*scaleFac;

				// P0
				marsHeightVertices[arrayIdx] = curX;
				marsHeightVertices[arrayIdx + 1] = heightScale*pTextureData[heightIdx];
				marsHeightVertices[arrayIdx + 2] = curZ;

				// P0 + dX
				marsHeightVertices[arrayIdx + 3] = curX+dX;
				marsHeightVertices[arrayIdx + 4] = heightScale*pTextureData[heightIdx + textureHeight];
				marsHeightVertices[arrayIdx + 5] = curZ;
				// P0 + dX + dZ
				marsHeightVertices[arrayIdx + 6] = curX+dX;
				marsHeightVertices[arrayIdx + 7] = heightScale*pTextureData[heightIdx + textureHeight + 1];
				marsHeightVertices[arrayIdx + 8] = curZ+dZ;
				// P0 + dZ
				marsHeightVertices[arrayIdx + 9] = curX;
				marsHeightVertices[arrayIdx + 10] = heightScale*pTextureData[heightIdx + 1];
				marsHeightVertices[arrayIdx + 11] = curZ+dZ;
			}
		}

		cout << "MBTEST DEBUGGING Vertices" << endl;
		debugArray(marsHeightVertices, 3, 100);

}

// number of cube textures
// 6 (one for each side of the cube)
static const int NUM_CUBE_TEXTURES = 6;

GLuint cubeTextureHandle;

GLenum  cube[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


const char *cubeTextureFileNames[ NUM_CUBE_TEXTURES ] = {  ".//planets//skybox//stars_pos_x.tga", 
                                                           ".//planets//skybox//stars_neg_x.tga", 
                                                           ".//planets//skybox//stars_pos_y.tga", 
                                                           ".//planets//skybox//stars_neg_y.tga", 
                                                           ".//planets//skybox//stars_pos_z.tga", 
                                                           ".//planets//skybox//stars_neg_z.tga" };

void loadCubeMap() 
{
   GLbyte *pTextureData;
   GLint textureWidth, textureHeight, textureComponents;
   GLenum textureFormat;

   // create texture handle for cube map
   glGenTextures( 1, &cubeTextureHandle );

   // bind texture object to texture handle
   glBindTexture( GL_TEXTURE_CUBE_MAP, cubeTextureHandle );

   // set texture filter
   GLenum minFilter = GL_LINEAR;
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, minFilter);
   GLenum magFilter = GL_LINEAR;
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, magFilter);
   // set texture wrap modes
   GLenum wrapMode = GL_CLAMP_TO_EDGE;
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);       

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   
   // Load Cube Map images
   for( int cubeTextureIndex = 0; cubeTextureIndex < NUM_CUBE_TEXTURES; cubeTextureIndex++ )
   { 
      // use a GLTools library function to read the pixel data from the .tga files
      pTextureData = gltReadTGABits( cubeTextureFileNames[ cubeTextureIndex ], &textureWidth, &textureHeight, &textureComponents, &textureFormat );
      assert( pTextureData != NULL );
      cout << "Texture " << cubeTextureFileNames[ cubeTextureIndex ] << " loaded. Width: " << textureWidth << " H: " << textureHeight << " C: " << textureComponents << " F: " << textureFormat << endl;

      // load texture object into GPU memory
      glTexImage2D( cube[ cubeTextureIndex ], 0, textureComponents, textureWidth, textureHeight, 0, textureFormat, GL_UNSIGNED_BYTE, pTextureData );
      
      // free CPU memory
      free(pTextureData);
   }
   glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void deleteTextures()
{
   // free memory for texture objects on GPU
   glDeleteTextures( NUM_TEXTURES, textureHandles );
   glDeleteTextures( 1, &cubeTextureHandle );
}

// ----------------------------------------------------------------------------------------------------

// RENDERING
// ----------------------------------------------------------------------------------------------------

void drawSphere(GLuint shaderProgramHandle, bool texCoordsFlag, bool tangentBinormalFlag, float radius, int numSlices, int numStacks)
{
   GLuint fvNormalHandle = glGetAttribLocation( shaderProgramHandle, "fvNormal" );
   GLuint fvTexCoordsHandle;
   GLuint fvTangentHandle;
   GLuint fvBinormalHandle;

   if (texCoordsFlag)
   {
      // TODO: Part 2
      // get the handle to the texture coordinates attribute of the vertex shader
      fvTexCoordsHandle = glGetAttribLocation(shaderProgramHandle, "fvTexCoords");
   }

   if (tangentBinormalFlag)
   {
      // TODO: Part 3
      // get the handles to the tangent and binormal attributes of the vertex shader
      fvTangentHandle = glGetAttribLocation( shaderProgramHandle, "fvTangent" );;
      fvBinormalHandle = glGetAttribLocation( shaderProgramHandle, "fvBinormal" );;
   }

   float dphi = 3.141592653589f / (float)numStacks;
   float dtheta = (2.0f * 3.141592653589f) / (float)numSlices;

   // TODO: Part 2
   // compute the step sizes for the updates of the two texture coordinates
   
   float ds = 1.0/numSlices;
   float dt = 1.0/numStacks;

   // TODO: Part 2
   // initialize the t coordinate
   float t = 1.0f;

   glBegin( GL_TRIANGLES );

   for( int i = 0; i < numStacks; i++ )
   {
      float phi = (float)i * dphi;
      float cosPhi = cos(phi);
      float sinPhi = sin(phi);
      float cosPhiPlusdphi = cos(phi + dphi);
      float sinPhiPlusdphi = sin(phi + dphi);

      // TODO: Part 2
      // initialize the s coordinate
      float s = 0.0f;

      for ( int j = 0; j < numSlices; j++ )
      {
         float theta = (float)j * dtheta;
         float cosTheta = (float)cos(theta);
         float sinTheta = (float)sin(theta);

         float thetaPlusdtheta = ((j+1) == numSlices) ? 0.0f : (float)(j+1) * dtheta;
         float cosThetaPlusdtheta = (float)cos( thetaPlusdtheta );
         float sinThetaPlusdtheta = (float)sin( thetaPlusdtheta );

         // TODO: compute four tangents
         float tx[4], ty[4], tz[4];

         tx[0] = -sinTheta;
         ty[0] = cosTheta;
         tz[0] = 0.0f;

		 // (theta, phi + dphi)
         tx[1] = -sinTheta;
         ty[1] = cosTheta;
         tz[1] = 0.0f;

		  //(theta + dtheta, phi + dphi)
         tx[2] = -sinThetaPlusdtheta;
         ty[2] = cosThetaPlusdtheta;
         tz[2] = 0.0f;

         tx[3] = -sinThetaPlusdtheta;
         ty[3] = cosThetaPlusdtheta;
         tz[3] = 0.0f;

         // TODO: compute four binormals
         float bx[4], by[4], bz[4];

         bx[0] = cosTheta * cosPhi;
         by[0] = cosPhi * sinTheta;
         bz[0] = -sinPhi;

		 // (theta, phi + dphi)
         bx[1] = cosTheta * cosPhiPlusdphi;
         by[1] = cosPhiPlusdphi * sinTheta;
         bz[1] = -sinPhiPlusdphi;

		 //(theta + dtheta, phi + dphi)
         bx[2] = cosThetaPlusdtheta * cosPhiPlusdphi;
         by[2] = cosPhiPlusdphi * sinThetaPlusdtheta;;
         bz[2] = -sinPhiPlusdphi;

		 // (theta + dtheta, phi)
         bx[3] = cosThetaPlusdtheta * cosPhi;
         by[3] = cosPhi * sinThetaPlusdtheta;
         bz[3] = -sinPhi;

         // define the four corners of the patch
         // compute four normals
         float nx[4], ny[4], nz[4];

         // (theta, phi)
         nx[0] = cosTheta * sinPhi;
         ny[0] = sinTheta * sinPhi;
         nz[0] = cosPhi;

         // (theta, phi + dphi)
         nx[1] = cosTheta * sinPhiPlusdphi;
         ny[1] = sinTheta * sinPhiPlusdphi;
         nz[1] = cosPhiPlusdphi;

         // (theta + dtheta, phi + dphi)
         nx[2] = cosThetaPlusdtheta * sinPhiPlusdphi;
         ny[2] = sinThetaPlusdtheta * sinPhiPlusdphi;
         nz[2] = cosPhiPlusdphi;

         // (theta + dtheta, phi)
         nx[3] = cosThetaPlusdtheta * sinPhi;
         ny[3] = sinThetaPlusdtheta * sinPhi;
         nz[3] = cosPhi;

         // compute four vertices
         float vx[4], vy[4], vz[4];
         for( int k = 0; k < 4; k++ )
         {
            vx[k] = radius * nx[k];
            vy[k] = radius * ny[k];
            vz[k] = radius * nz[k];
         }

         // first triangle (0, 1, 3)
         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 0 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[0], by[0], bz[0]);
			 glVertexAttrib3f( fvTangentHandle, tx[0], ty[0], tz[0]);
         }
         glVertexAttrib3f( fvNormalHandle, nx[0], ny[0], nz[0]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            glVertexAttrib2f( fvTexCoordsHandle, s, t );

         }
         glVertex3f( vx[0], vy[0], vz[0] );

         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 1 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[1], by[1], bz[1]);
			 glVertexAttrib3f( fvTangentHandle, tx[1], ty[1], tz[1]);
         }
         glVertexAttrib3f( fvNormalHandle, nx[1], ny[1], nz[1]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            glVertexAttrib2f( fvTexCoordsHandle, s, t-dt );
         }
         glVertex3f( vx[1], vy[1], vz[1] );
         
         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 3 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[3], by[3], bz[3]);
			 glVertexAttrib3f( fvTangentHandle, tx[3], ty[3], tz[3]);
         }
		 
         glVertexAttrib3f( fvNormalHandle, nx[3], ny[3], nz[3]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            glVertexAttrib2f( fvTexCoordsHandle, s + ds, t );
         }
         glVertex3f( vx[3], vy[3], vz[3] );
         
         // second triangle (1, 2, 3)
         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 1 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[1], by[1], bz[1]);
			 glVertexAttrib3f( fvTangentHandle, tx[1], ty[1], tz[1]);
         }
         glVertexAttrib3f( fvNormalHandle, nx[1], ny[1], nz[1]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            glVertexAttrib2f( fvTexCoordsHandle, s, t-dt );
         }
         glVertex3f( vx[1], vy[1], vz[1] );
         
         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 2 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[2], by[2], bz[2]);
			 glVertexAttrib3f( fvTangentHandle, tx[2], ty[2], tz[2]);
         }
         glVertexAttrib3f( fvNormalHandle, nx[2], ny[2], nz[2]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            glVertexAttrib2f( fvTexCoordsHandle, s+ds, t-dt );
         }
         glVertex3f( vx[2], vy[2], vz[2] );

         if (tangentBinormalFlag)
         {
            // TODO: Part 3
            // send the tangent and binormal attributes for vertex 3 to the vertex shader
			 glVertexAttrib3f( fvBinormalHandle, bx[3], by[3], bz[3]);
			 glVertexAttrib3f( fvTangentHandle, tx[3], ty[3], tz[3]);
         }
         glVertexAttrib3f( fvNormalHandle, nx[3], ny[3], nz[3]);
         if (texCoordsFlag)
         {
            // TODO: Part 2
            // send the texture coordinates attribute for vertex 3 to the vertex shader
			 glVertexAttrib2f( fvTexCoordsHandle, s+ds, t );
         }
         glVertex3f( vx[3], vy[3], vz[3] );

         // TODO: Part 2 
         s = s+ds;
      }

      // TODO: Part 2
      t = t-dt;
   }

   glEnd();
}

void drawCube( float radius )
{
   glBegin( GL_TRIANGLES );
   
   /////////////////////////////////////////////
   // Top of cube
   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, radius);

   ////////////////////////////////////////////
   // Bottom of cube
   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(-radius, -radius, -radius);

   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(radius, -radius, -radius);

   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(radius, -radius, radius);

   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(-radius, -radius, radius);

   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(-radius, -radius, -radius);

   glNormal3f(0.0f, -radius, 0.0f);
   glVertex3f(radius, -radius, radius);

   ///////////////////////////////////////////
   // Left side of cube
   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, radius, radius);

   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, -radius, -radius);

   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, radius, radius);

   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, -radius, -radius);

   glNormal3f(-radius, 0.0f, 0.0f);
   glVertex3f(-radius, -radius, radius);

   // Right side of cube
   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, -radius, -radius);

   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, radius, -radius);

   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, -radius, radius);

   glNormal3f(radius, 0.0f, 0.0f);
   glVertex3f(radius, -radius, -radius);

   // Front and Back
   // Front
   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(radius, -radius, radius);

   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(radius, radius, radius);

   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(-radius, radius, radius);

   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(-radius, radius, radius);

   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(-radius, -radius, radius);

   glNormal3f(0.0f, 0.0f, radius);
   glVertex3f(radius, -radius, radius);

   // Back
   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(radius, -radius, -radius);

   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(-radius, -radius, -radius);

   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(radius, radius, -radius);

   glNormal3f(0.0f, 0.0f, -radius);
   glVertex3f(radius, -radius, -radius);
   
   glEnd();
}

void drawCubeFace(float radius)
{
   glBegin( GL_TRIANGLES );
   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(radius, radius, radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, -radius);

   glNormal3f(0.0f, radius, 0.0f);
   glVertex3f(-radius, radius, radius);

   glEnd();
}

void debugArray (GLfloat *array1, int index, int elements)
{
	int totalCells = index*elements;
	for (int i=0; i < totalCells; i+= index){
		if (i%4 == 0)
		{
			cout << endl;
		}
		cout << "(";
		for (int j=0; j < index; j++)
		{
			cout << array1[i+j];
			if (j < (index - 1))
			{
				cout << ", ";
			}
		}
		cout << ") ";
		
	}
}
void drawTerrain2( DrawMode drawMode = DRAW_PHONG )
{
	// enble and specify pointers to vertex arrays
	GLuint shaderProgramHandle;
	GLuint fvNormalHandle = glGetAttribLocation( shaderProgramHandle, "fvNormal" );
	switch( drawMode )
   {
   case DRAW_PHONG:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_PHONG ];
         break;
      }
   case DRAW_COLOR_MAP:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_COLOR_MAP ];
         break;
      }
	}
	glDisable(GL_CULL_FACE);
    //glEnableClientState(GL_NORMAL_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
	//glColorPointer(3, GL_FLOAT, 0, marsHeightColors);

    //glNormalPointer(GL_FLOAT, 0, marsHeightNormals);

	glEnableVertexAttribArray(fvNormalHandle);
    glVertexAttribPointer(fvNormalHandle, 3, GL_FLOAT, GL_TRUE, 0, marsHeightNormals);
	
	//glEnableVertexAttribArray(colorMapHandle);
    //glVertexAttribPointer(colorMapHandle, 3, GL_FLOAT, GL_TRUE, 0, marsHeightColors);

	glVertexPointer(3, GL_FLOAT, 0, marsHeightVertices);
	glEnableClientState(GL_VERTEX_ARRAY);



    glPushMatrix();
    //glTranslatef(2, 2, 0);  
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE ); // move to upper-right

    glDrawArrays(GL_QUADS, 0, quadsToRender);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    //glDisableClientState(GL_COLOR_ARRAY);
	glDisableVertexAttribArray(fvNormalHandle);
	//glDisableVertexAttribArray(colorMapHandle);
    //glDisableClientState(GL_NORMAL_ARRAY);
	glEnable(GL_CULL_FACE);
}

void drawPlanet(   DrawMode drawMode = DRAW_PHONG, float planetRadius = 1.0f, 
                   Textures colorMapTexture = TEXTURE_EARTH_COLOR_MAP, Textures normalMapTexture = TEXTURE_EARTH_NORMAL_MAP,
                   bool cloudMapFlag = false, float cloudRadius = 1.05f, 
                   Textures cloudMapTexture = TEXTURE_EARTH_CLOUD_MAP, Textures cloudTransMapTexture = TEXTURE_EARTH_CLOUD_TRANS_MAP) 
{
   // shader handle
   GLuint shaderProgramHandle;
   // light handle
   GLuint fvLightWorldCoordsHandle;
   // projection, viewing and model matrix handles
   GLuint fmProjHandle;
   GLuint fmViewHandle;
   GLuint fmModelHandle;
   // color handles 
   GLuint fvAmbientHandle;
   GLuint fvDiffuseHandle;
   GLuint fvSpecularHandle;
   GLuint fSpecularPowerHandle;
   GLuint fvBaseColorHandle;

   // sets the shader program for color or bump map
   switch( drawMode )
   {
   case DRAW_PHONG:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_PHONG ];
         break;
      }
   case DRAW_COLOR_MAP:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_COLOR_MAP ];
         break;
      }
   case DRAW_MARS:
	   {
		  shaderProgramHandle = shaderProgramHandles[ SHADER_MARS ];
	   }
	  /*
   case DRAW_BUMP_MAP:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_BUMP_MAP ];
         break;
      } */
   }
   
   glUseProgram( shaderProgramHandle );

   // handles to uniform variables for the shading program
   fvLightWorldCoordsHandle = glGetUniformLocation(shaderProgramHandle, "fvLightWorldCoords");
   fmProjHandle = glGetUniformLocation( shaderProgramHandle, "fmProj" );
   fmViewHandle = glGetUniformLocation( shaderProgramHandle, "fmView" );
   fmModelHandle = glGetUniformLocation( shaderProgramHandle, "fmModel" );
   fvAmbientHandle = glGetUniformLocation(shaderProgramHandle, "fvAmbient");
   fvDiffuseHandle = glGetUniformLocation(shaderProgramHandle, "fvDiffuse");
   fvSpecularHandle = glGetUniformLocation(shaderProgramHandle, "fvSpecular");
   fSpecularPowerHandle = glGetUniformLocation(shaderProgramHandle, "fSpecularPower");
   fvBaseColorHandle = glGetUniformLocation(shaderProgramHandle, "fvBaseColor");

   glUniform3fv( fvLightWorldCoordsHandle, 1, fvLightWorldCoords );
   glUniformMatrix4fv( fmProjHandle, 1, false, fmProj );
   glUniformMatrix4fv( fmViewHandle, 1, false, fmView );
   glUniformMatrix4fv( fmModelHandle, 1, false, fmModel );
   glUniform4fv(fvAmbientHandle, 1, fvAmbient);
   glUniform4fv(fvSpecularHandle, 1, fvSpecular);
   glUniform4fv(fvDiffuseHandle, 1, fvDiffuse);
   glUniform1fv(fSpecularPowerHandle, 1, fSpecularPower);
   glUniform4fv( fvBaseColorHandle, 1, fvBaseColor);

   if ( ( drawMode == DRAW_COLOR_MAP ) || ( drawMode == DRAW_BUMP_MAP ) )
   {
      // color map (only used in colormap shader)
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, textureHandles[ colorMapTexture ] );
      GLint colorMapHandle = glGetUniformLocation( shaderProgramHandle, "colorMap" );
      glUniform1i( colorMapHandle, 0 );
   }
   
   if ( drawMode == DRAW_BUMP_MAP ) // bumpmap
   {
      // normal map (only used in bumpmap shader
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, textureHandles[ normalMapTexture ] );
      GLint normalMapHandle = glGetUniformLocation( shaderProgramHandle, "normalMap" );
      glUniform1i( normalMapHandle, 1);
   }

   // draw planet
   switch ( drawMode )
   {
      case DRAW_PHONG:
         {
            drawSphere( shaderProgramHandle, false, false, planetRadius, 100.0, 100.0 );
            break;
         }
      case DRAW_COLOR_MAP:
         {
            drawSphere( shaderProgramHandle, true, false, planetRadius, 100.0, 100.0 );
            break;
         }
      case DRAW_BUMP_MAP:
         {
            drawSphere( shaderProgramHandle, true, true, planetRadius, 100.0, 100.0 );
            break;
         }
   }

   // draw cloud map
   if (cloudMapFlag)
   {
      // sets the shader program for color or bump map
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

      shaderProgramHandle = shaderProgramHandles[ SHADER_CLOUD_MAP ]; 
      glUseProgram( shaderProgramHandle );

      // Handles to uniform variables for the shading program
      fvLightWorldCoordsHandle = glGetUniformLocation(shaderProgramHandle, "fvLightWorldCoords");
      fmProjHandle = glGetUniformLocation( shaderProgramHandle, "fmProj" );
      fmViewHandle = glGetUniformLocation( shaderProgramHandle, "fmView" );
      fmModelHandle = glGetUniformLocation( shaderProgramHandle, "fmModel" );
      fvAmbientHandle = glGetUniformLocation(shaderProgramHandle, "fvAmbient");
      fvDiffuseHandle = glGetUniformLocation(shaderProgramHandle, "fvDiffuse");
      fvSpecularHandle = glGetUniformLocation(shaderProgramHandle, "fvSpecular");
      fSpecularPowerHandle = glGetUniformLocation(shaderProgramHandle, "fSpecularPower");
      fvBaseColorHandle = glGetUniformLocation(shaderProgramHandle, "fvBaseColor");

      glUniform3fv( fvLightWorldCoordsHandle, 1, fvLightWorldCoords );
      glUniformMatrix4fv( fmProjHandle, 1, false, fmProj );
      glUniformMatrix4fv( fmViewHandle, 1, false, fmView );
      glUniformMatrix4fv( fmModelHandle, 1, false, fmModel );
      glUniform4fv(fvAmbientHandle, 1, fvAmbient);
      glUniform4fv(fvSpecularHandle, 1, fvSpecular);
      glUniform4fv(fvDiffuseHandle, 1, fvDiffuse);
      glUniform1fv(fSpecularPowerHandle, 1, fSpecularPower);
      glUniform4fv( fvBaseColorHandle, 1, fvBaseColor);

      // cloud map
      glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_2D, textureHandles[ cloudMapTexture ] );
      GLint cloudMapHandle = glGetUniformLocation( shaderProgramHandle, "cloudMap" );
      glUniform1i( cloudMapHandle, 2);
      // cloud transparent map
      glActiveTexture( GL_TEXTURE3 );
      glBindTexture( GL_TEXTURE_2D, textureHandles[ cloudTransMapTexture ] );
      GLint cloudTransMapHandle = glGetUniformLocation( shaderProgramHandle, "cloudTransMap" );
      glUniform1i( cloudTransMapHandle, 3);

      drawSphere( shaderProgramHandle, true, false, cloudRadius, 100.0, 100.0);

      glDisable( GL_BLEND );
   }
}

/* MBTEST */
void drawTerrainWrap(   DrawMode drawMode = DRAW_MARS,
                   Textures colorMapTexture = TEXTURE_MARS_COLOR_MAP, 
				   Textures normalMapTexture = TEXTURE_MARS_NORMAL_MAP ) 
{
	glUseProgram(0);
	// shader handle
   GLuint shaderProgramHandle;
   // light handle
   GLuint fvLightWorldCoordsHandle;
   // projection, viewing and model matrix handles
   GLuint fmProjHandle;
   GLuint fmViewHandle;
   GLuint fmModelHandle;
   // color handles 
   GLuint fvAmbientHandle;
   GLuint fvDiffuseHandle;
   GLuint fvSpecularHandle;
   GLuint fSpecularPowerHandle;
   GLuint fvBaseColorHandle;

   GLuint fvEyeHandle;

   // sets the shader program for color or bump map
   switch( drawMode )
   {
   case DRAW_PHONG:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_PHONG ];
         break;
      }
   case DRAW_COLOR_MAP:
      {
         shaderProgramHandle = shaderProgramHandles[ SHADER_COLOR_MAP ];
         break;
      }
	case DRAW_MARS:
	   {
		  shaderProgramHandle = shaderProgramHandles[ SHADER_MARS ];
	   }
   }
   
   glUseProgram( shaderProgramHandle );

   // handles to uniform variables for the shading program
   fvLightWorldCoordsHandle = glGetUniformLocation(shaderProgramHandle, "fvLightWorldCoords");
   fmProjHandle = glGetUniformLocation( shaderProgramHandle, "fmProj" );
   fmViewHandle = glGetUniformLocation( shaderProgramHandle, "fmView" );
   fmModelHandle = glGetUniformLocation( shaderProgramHandle, "fmModel" );
   fvAmbientHandle = glGetUniformLocation(shaderProgramHandle, "fvAmbient");
   fvDiffuseHandle = glGetUniformLocation(shaderProgramHandle, "fvDiffuse");
   fvSpecularHandle = glGetUniformLocation(shaderProgramHandle, "fvSpecular");
   fSpecularPowerHandle = glGetUniformLocation(shaderProgramHandle, "fSpecularPower");
   fvBaseColorHandle = glGetUniformLocation(shaderProgramHandle, "fvBaseColor");
   GLuint fvNormalHandle = glGetAttribLocation( shaderProgramHandle, "fvNormal" );
   GLuint fvTexCoordsHandle = glGetAttribLocation( shaderProgramHandle, "fvTexCoords");

   fvEyeHandle = glGetAttribLocation(shaderProgramHandle, "fvEye");

   glGetFloatv(GL_MODELVIEW_MATRIX, fmModel);

   glUniform3fv( fvLightWorldCoordsHandle, 1, fvLightWorldCoords );
   glUniformMatrix4fv( fmProjHandle, 1, false, fmProj );
   glUniformMatrix4fv( fmViewHandle, 1, false, fmView );
   glUniformMatrix4fv( fmModelHandle, 1, false, fmModel );
   glUniform4fv(fvAmbientHandle, 1, fvAmbient);
   glUniform4fv(fvSpecularHandle, 1, fvSpecular);
   glUniform4fv(fvDiffuseHandle, 1, fvDiffuse);
   glUniform1fv(fSpecularPowerHandle, 1, fSpecularPower);
   glUniform4fv( fvBaseColorHandle, 1, fvBaseColor);


   //glUniform3fv( fvEyeHandle, 1, fvEye );

      // color map (only used in colormap shader)
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, textureHandles[ colorMapTexture ] );
      GLint colorMapHandle = glGetUniformLocation( shaderProgramHandle, "colorMap" );
      glUniform1i( colorMapHandle, 0 );
   
	glDisable(GL_CULL_FACE);
 
	glEnableVertexAttribArray(fvNormalHandle);
    glVertexAttribPointer(fvNormalHandle, 3, GL_FLOAT, GL_TRUE, 0, marsHeightNormals);
	/*
	glEnableVertexAttribArray(colorMapHandle);
    glVertexAttribPointer(colorMapHandle, 3, GL_FLOAT, GL_TRUE, 0, marsHeightColors);*/

	glEnableVertexAttribArray(fvTexCoordsHandle);
	glVertexAttribPointer( fvTexCoordsHandle, 2, GL_UNSIGNED_BYTE, GL_TRUE, 0, marsHeightIndices);

	glVertexPointer(3, GL_FLOAT, 0, marsHeightVertices);
	glEnableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );

	/* MBTEST hacky */
	quadsToRender = 1024*512;
    glDrawArrays(GL_QUADS, 0, quadsToRender);
	//glDrawElements(GL_QUADS, quadsToRender, GL_UNSIGNED_BYTE, marsHeightIndices);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	glDisableVertexAttribArray(fvNormalHandle);
	//glDisableVertexAttribArray(colorMapHandle);
	glDisableVertexAttribArray(fvTexCoordsHandle);
	glEnable(GL_CULL_FACE);
}

void drawQuadTest()
{
    // vertex coords array
GLfloat vertices[] = {1,1,1,  -1,1,1,  -1,-1,1,  1,-1,1,        // v0-v1-v2-v3
                      1,1,1,  1,-1,1,  1,-1,-1,  1,1,-1,        // v0-v3-v4-v5
                      1,1,1,  1,1,-1,  -1,1,-1,  -1,1,1,        // v0-v5-v6-v1
                      -1,1,1,  -1,1,-1,  -1,-1,-1,  -1,-1,1,    // v1-v6-v7-v2
                      -1,-1,-1,  1,-1,-1,  1,-1,1,  -1,-1,1,    // v7-v4-v3-v2
                      1,-1,-1,  -1,-1,-1,  -1,1,-1,  1,1,-1};   // v4-v7-v6-v5

// normal array
GLfloat normals[] = {0,0,1,  0,0,1,  0,0,1,  0,0,1,             // v0-v1-v2-v3
                     1,0,0,  1,0,0,  1,0,0, 1,0,0,              // v0-v3-v4-v5
                     0,1,0,  0,1,0,  0,1,0, 0,1,0,              // v0-v5-v6-v1
                     -1,0,0,  -1,0,0, -1,0,0,  -1,0,0,          // v1-v6-v7-v2
                     0,-1,0,  0,-1,0,  0,-1,0,  0,-1,0,         // v7-v4-v3-v2
                     0,0,-1,  0,0,-1,  0,0,-1,  0,0,-1};        // v4-v7-v6-v5

// color array
GLfloat colors[] = {1,1,1,  1,1,0,  1,0,0,  1,0,1,              // v0-v1-v2-v3
                    1,1,1,  1,0,1,  0,0,1,  0,1,1,              // v0-v3-v4-v5
                    1,1,1,  0,1,1,  0,1,0,  1,1,0,              // v0-v5-v6-v1
                    1,1,0,  0,1,0,  0,0,0,  1,0,0,              // v1-v6-v7-v2
                    0,0,0,  0,0,1,  1,0,1,  1,0,0,              // v7-v4-v3-v2
                    0,0,1,  0,0,0,  0,1,0,  0,1,1};             // v4-v7-v6-v5
	
	// enble and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals);
    glColorPointer(3, GL_FLOAT, 0, colors);
    glVertexPointer(3, GL_FLOAT, 0, vertices);

	 
    glPushMatrix();

    glDrawArrays(GL_QUADS, 0, 24);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}




void drawScene()
{
   // get model matrix M
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glPushMatrix();

   //glRotated( earthRotAngle, 0.0f, 0.0f, 1.0f );
   // draw earth at the origin
   //glGetFloatv(GL_MODELVIEW_MATRIX, fmModel);
   /*
   drawPlanet( activeDrawMode, 0.2f, TEXTURE_EARTH_COLOR_MAP, TEXTURE_EARTH_NORMAL_MAP, 
               drawClouds, 0.205f, TEXTURE_EARTH_CLOUD_MAP, TEXTURE_EARTH_CLOUD_TRANS_MAP );
			   */

   //drawQuadTest();

   drawTerrainWrap();

   glPopMatrix();
   /*
   glPushMatrix();

      glRotatef( sunRotAngle, 0.0f, 0.0f, 1.0f);
      glTranslatef( 2.0f, 0.0f, 0.0f );

      // draw sun
      glGetFloatv(GL_MODELVIEW_MATRIX, fmModel);
      if ( (activeDrawMode == DRAW_PHONG) || (activeDrawMode == DRAW_COLOR_MAP) )
      {
         drawPlanet( activeDrawMode, 0.3f, TEXTURE_SUN_COLOR_MAP );
      }
      else
      {
         drawPlanet( DRAW_COLOR_MAP, 0.3f, TEXTURE_SUN_COLOR_MAP );
      }

   glPopMatrix();

   glPushMatrix();
   
      glRotatef( moonRotAngle, 1.0f, 0.0f, 0.0f );
      glTranslatef( 0.0f, 0.5f, 0.0f );
      
      // draw moon
      glGetFloatv(GL_MODELVIEW_MATRIX, fmModel);
      drawPlanet( activeDrawMode, 0.1f, TEXTURE_MOON_COLOR_MAP, TEXTURE_MOON_NORMAL_MAP );

   glPopMatrix();
   */
}

void drawTerrain() 
{
	// get model matrix M
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glPushMatrix();

   // draw earth at the origin
   glGetFloatv(GL_MODELVIEW_MATRIX, fmModel);
   drawPlanet( activeDrawMode, 0.2f, TEXTURE_MARS_COLOR_MAP, TEXTURE_MARS_NORMAL_MAP, 
               drawClouds, 0.205f, TEXTURE_EARTH_CLOUD_MAP, TEXTURE_EARTH_CLOUD_TRANS_MAP );
   glPopMatrix();
}

void render()
{
	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Move the camera forward for the landspeeder
	cam.slide(0.0f, 0.0f, (-0.001)*throttle);

	cam.apply();

	//landspeeder.render();

   // get projection matrix P
   glMatrixMode(GL_PROJECTION);
   glGetFloatv(GL_PROJECTION_MATRIX, fmProj);

   // get viewing matrix V
	glMatrixMode(GL_MODELVIEW);
   glGetFloatv(GL_MODELVIEW_MATRIX, fmView);


   // draw skyBox
   if (drawSkyBox)
   {
      glDisable(GL_CULL_FACE);
      GLuint shaderProgramHandle = shaderProgramHandles[ SHADER_SKYBOX ]; 
      
      glUseProgram( shaderProgramHandle );

      GLuint fmProjHandle = glGetUniformLocation( shaderProgramHandle, "fmProj" );
      GLuint fmViewHandle = glGetUniformLocation( shaderProgramHandle, "fmView" );
      glUniformMatrix4fv( fmProjHandle, 1, false, fmProj );
      glUniformMatrix4fv( fmViewHandle, 1, false, fmView );


      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_CUBE_MAP, cubeTextureHandle );
      GLint cubeMapHandle = glGetUniformLocation( shaderProgramHandle, "cubeMap" );
      glUniform1i( cubeMapHandle, 0);

      drawCube( 20.0 );
      glEnable(GL_CULL_FACE);
   }
   // Draws Mars
   glLightfv(GL_LIGHT1, GL_POSITION, sunlightD);

  glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient2);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular2);
  glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse2);
   drawTerrain();

   drawScene();
   //glDisable(GL_LIGHTING);
   checkGLErrors();
}

// ----------------------------------------------------------------------------------------------------

// GLUT CALLBACKS
// ----------------------------------------------------------------------------------------------------

// _____________________________________________________
//|														           |
//|	 reshape														  |
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
	
	glViewport( 0, 0, w, h );
   
   cam.setAspectRatio( (float)w / (float)h );
}

// _____________________________________________________
//|														           |
//|	 display											           |
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
//|														           |
//|	 keyboard											        |
//|_____________________________________________________|
///
///  Whenever a key on the keyboard is pressed, 
///  a "keyboard" event is generated and GLUT is told 
///  to call this keyboard callback function to handle it
///  appropriately.

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
      {
		   exit(0);
		   break;
      }
   case 'a':
      {
         animate = animate?false:true;
         break;
      }
   case 'd':
      {
         // TODO: Part 2
         // uncommment this code to test your color mapping shader
		  
         switch( activeDrawMode )
         {
         case DRAW_PHONG:
            {
               activeDrawMode = DRAW_COLOR_MAP;
               cout << "The active shader is set to color map." << endl;
               break;
            }
         case DRAW_COLOR_MAP:
            {
               activeDrawMode = DRAW_PHONG;
               cout << "The active shader is set to phong." << endl;
               break;
            }
         } 

         break;
      }
   case 'c':
      {
         // TODO: Part 4
         drawClouds = drawClouds?false:true;
         if ( drawClouds )
         {
            cout << "The earth is drawn with clouds." << endl;
         }
         else
         {
            cout << "The earth is drawn without clouds." << endl;
         }
         break;
      }
   case 's':
      {
         // TODO: Part 5
         // uncomment the following line to test your skybox shader
		  /*
         drawSkyBox = drawSkyBox?false:true;
         if ( drawSkyBox )
         {
            cout << "A skybox is drawn." << endl;
         }
         else 
         {
            cout << "No skybox is drawn." << endl;  
         }
         break;
		 */

		  cam.roll( -2.0f );
		  break;
      }
   case 'f':
	   {
		   cam.roll( 2.0f );
		   break;
	   }
	}

	// pitch
	if (key == 'i') cam.pitch( 2.0f );
	if (key == 'k') cam.pitch( -2.0f );
	// yaw
	if (key == 't') cam.yaw( -2.0f );
	if (key == 'g') cam.yaw( 2.0f );
	// roll
	if (key == 'l') cam.roll( 2.0f );
	if (key == 'j') cam.roll( -2.0f );	
	// zoom
	if (key == 'y') cam.slide( 0.0f, 0.0f, -0.05f );
	if (key == 'h') cam.slide( 0.0f, 0.0f, 0.05f );

	// scan
	if (key == 'q') cam.slide(-0.5f, 0.0f, 0.0f);
	if (key == 'w') cam.slide(0.5f, 0.0f, 0.0f);

	// Control throttle
	if (key == 'a') {
		if (throttle < MAX_THROTTLE) throttle++;
	}
	if (key == 'z') {
		if (throttle > 0) throttle--;
	}
	glutPostRedisplay();
}

// _____________________________________________________
//|														           |
//|	 mouse											           |
//|_____________________________________________________|
///
///  Whenever a mouse button is clicked, a "mouse" event 
///  is generated and GLUT calls this mouse callback to 
///  handle it appropriately.

void mouse(int button, int state, int x, int y)
{
   // add mouse event handling code here

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
//|														           |
//|	 motion											           |
//|_____________________________________________________|
///
///  Moving the mouse generates a "motion" event that can be 
///  handled in this motion callback function. Using this
///  in combination with the "mouse" callback will allow us
///  to interact with the scene using the mouse.

void motion(int x, int y)
{
   // add mouse motion handling code here
	if(moving)
	{

		//cout << "("<<x<<","<<y<<")"<<endl;
		float dx = clamp(camMouseFine*(beginx-x), -1, 1);
		float dy = clamp(camMouseFine*(beginy-y), -1, 1);
		cam.yaw(asin(dx));
		cam.pitch(asin(dy));
		beginx = x;
		beginy = y;
	}
	glutPostRedisplay();

}

// _____________________________________________________
//|														           |
//|	 timer											           |
//|_____________________________________________________|
void timerRotMoon(int)
{
   if(animate)
   {
      moonRotAngle += 0.5;
      if(   moonRotAngle > 360)
            moonRotAngle -= 360;
   }
   glutTimerFunc( 2.0, timerRotMoon, 0);
   glutPostRedisplay();
}

void timerRotSun(int)
{
   if(animate)
   {
      sunRotAngle += 0.5;
      if(   sunRotAngle > 360)
            sunRotAngle -= 360;
   }
   glutTimerFunc( 10.0, timerRotSun, 0);
   glutPostRedisplay();
}

void timerRotEarth(int)
{
   if(animate)
   {
      earthRotAngle += 0.5;
      if(   earthRotAngle > 360)
         earthRotAngle -= 360;
   }
   glutTimerFunc( 10.0, timerRotEarth, 0);
   glutPostRedisplay();
}

// ----------------------------------------------------------------------------------------------------


// INITIALIZATIONS
// ----------------------------------------------------------------------------------------------------

void initsGLUTState( )
{
	// use RGBA pixel channels, double buffering, and depth buffering
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// choose initial window size 
	glutInitWindowSize( INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT );

	// create a window
	glutCreateWindow( "Texture Mapping" );

	// register the callback functions
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc( motion );
   
	// register timer functions needed for animation
   glutTimerFunc( 2.0, timerRotMoon, 0);
   glutTimerFunc( 10.0, timerRotEarth, 0);
   glutTimerFunc( 10.0, timerRotSun, 0);
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

   // init camera parameters
   cam.setFov( FOV );
	cam.setNearPlane( NEAR_PLANE );
	cam.setFarPlane( FAR_PLANE );
	cam.set( camInitCenter[0], camInitCenter[1], camInitCenter[2], 
				0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f );

	moving = 0;
	//trackball(quatCurr, 0.0, 0.0, 0.0, 0.0);
	//landspeeder.loadOBJ(LANDSPEEDER_FILE);
	//glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT1);

	checkGLErrors();
}

void initGLSLState()
{
   for ( int shaderIndex = 0; shaderIndex < NUM_SHADERS; ++shaderIndex )
   {
      GLuint shaderProgramHandle = compileShaders( shaderProgramFileNames[ shaderIndex ][ 0 ], shaderProgramFileNames[ shaderIndex ][ 1 ] );
      shaderProgramHandles[ shaderIndex ] = shaderProgramHandle;
   }
   loadTextures();
   loadCubeMap();
   checkGLErrors();
}

void cleanGLSLState()
{
   for ( int shaderIndex = 0; shaderIndex < NUM_SHADERS; shaderIndex++ )
   {
      glDeleteProgram( shaderProgramHandles[ shaderIndex ] );
   }
   deleteTextures();
   checkGLErrors();
}

// ----------------------------------------------------------------------------------------------------

// MAIN
// ----------------------------------------------------------------------------------------------------

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

      // initialize GLSL states (shaders)
      initGLSLState();

		// enter the GLUT main (event) loop
		glutMainLoop();

      // cleanup GLSL states (shaders)
      cleanGLSLState();

		return 0;
	}
	catch (exception& e) {
		cout << "Exception caught: " << e.what() << endl;
		return -1;
	}
}

// ----------------------------------------------------------------------------------------------------