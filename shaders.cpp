#include <fstream>
#include <string>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef __MAC__
#	include <GLUT/glut.h>
#else
#  define FREEGLUT_STATIC
#  include <GL/glut.h>
#endif


#include "shaders.h"

using namespace std;

// H E L P E R    F U N C T I O N S ////////////////////////////////////

// function to read a text file 
static void readTextFile(const char *fn, string& data) 
{
	// Sets ios::binary bit to prevent end of line translation, so that the
	// number of bytes we read equals file size
	ifstream ifs(fn, ios::in | ios::binary);
	if (ifs.is_open()) {
		// Sets bits to report IO error using exception
		ifs.exceptions(ios::eofbit | ios::failbit | ios::badbit);
		ifs.seekg(0, ios::end);
		size_t len = ifs.tellg();
		data.resize(len);
		ifs.seekg(0, ios::beg);
		ifs.read(&data[0], len);
		ifs.close();
	}
	else {
		throw runtime_error(string("Cannot open file ") + fn);
	}
}

static void printInfoLog(GLuint obj, const char* filename)
{
	GLint infologLength = 0;
  	GLint charsWritten  = 0;
  	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
  	if (infologLength <= 0)
		return;
	string infoLog(infologLength, 0);
  	glGetInfoLogARB(obj, infologLength, &charsWritten, &infoLog[0]);
  	std::cerr << "\nMsg [" << filename << "]:\n--------------\n\n" << infoLog;
}

GLuint compileShaders(const char * vertexShaderFileName, const char * fragmentShaderFileName ) 
{
	// Read shader programs from text files. Exceptions will be thrown for errors
	string vertexShaderString, fragmentShaderString;
	readTextFile( vertexShaderFileName, vertexShaderString );
	readTextFile( fragmentShaderFileName, fragmentShaderString );

	// create handles for shaders
	GLuint vertexShaderHandle = glCreateShader( GL_VERTEX_SHADER );
	GLuint fragmentShaderHandle = glCreateShader( GL_FRAGMENT_SHADER );

	// set the source code fo the shaders
	const char * constVertexShaderString = vertexShaderString.c_str();
	const char * constFragmentShaderString = fragmentShaderString.c_str();
	glShaderSource( vertexShaderHandle, 1, &constVertexShaderString, NULL );
	glShaderSource( fragmentShaderHandle, 1, &constFragmentShaderString, NULL );

	// compile the shaders
	glCompileShader( vertexShaderHandle );
	glCompileShader( fragmentShaderHandle );

	// get compile status
	GLint vsCompiled=0, fsCompiled=0;
	glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &vsCompiled);
	glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &fsCompiled);

	printInfoLog(vertexShaderHandle, vertexShaderFileName);
	printInfoLog(fragmentShaderHandle, fragmentShaderFileName);

	if (!vsCompiled || !fsCompiled) {
		if (!vsCompiled)
			cerr << "Error: could not compile vertex shader program " << vertexShaderFileName << endl;
		if (!fsCompiled)
			cerr << "Error: could not compile fragment shader program " << fragmentShaderFileName << endl;
		throw runtime_error("Shaders failed to compile.");
	}


	// create a program container and attach the shaders to it
	GLuint programHandle = glCreateProgram( );
	glAttachShader( programHandle, vertexShaderHandle );
	glAttachShader( programHandle, fragmentShaderHandle );

	// link to this program
	glLinkProgram( programHandle );

	// load and use this program
	glUseProgram( programHandle );
	printInfoLog(programHandle, "");

	return programHandle;
}
