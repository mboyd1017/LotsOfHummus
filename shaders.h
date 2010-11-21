#ifndef __SHADERS_H__
#define __SHADERS_H__

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef __MAC__
#	include <GLUT/glut.h>
#else
#  define FREEGLUT_STATIC
#  include <GL/glut.h>
#endif
GLuint compileShaders(const char * vertexShaderFileName, const char * fragmentShaderFileName );

#endif
