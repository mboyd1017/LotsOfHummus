#ifndef CAMERA_H
#define CAMERA_H

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef __MAC__
#	include <GLUT/glut.h>
#else
#  define FREEGLUT_STATIC
#  include <GL/glut.h>
#endif

#include "Vector.h"

class Camera {

public:
	// ctor
	Camera( float fov = 45.0, float aspectRatio = 1.0, float nearPlane = 0.01f, float farPlane = 100.0f );
  
	// same functionality as gluLookAt
   void set(	float eyeX, float eyeY, float eyeZ, 
					float lookX, float lookY, float lookZ, 
					float upX, float upY, float upZ );
	// sets the aspect ratio of the camera
	void setAspectRatio( float aspectRatio );
	// sets the field of view parameter of the camera
	void setFov( float fov );
	// sets the near plane (clipping) of the camera
	void setNearPlane( float nearPlane );
	// sets the far plane (clipping) of the camera
	void setFarPlane( float farPlane );

	// roll the camera 
	void roll( float rollAngle );

	// pitch the camera
	void pitch( float pitchAngle );

	// yaw the camera
	void yaw( float yawAngle );

	// slides the camera
	// a negative dN: zoom in
	// a positive dN: zoom out
	// dU and DV allow pan functionality
	void slide( float dU, float dV, float dN );

	// sends the corresponding OpenGL 
	// calls to set the camera
	void apply() const;

	   // camera position
   CVector<float,3> m_eye;

private:
   // perspective projection 
   // parameters
   float m_fov;
   float m_aspectRatio;
   float m_nearPlane;
   float m_farPlane;



   // camera axis
	CVector<float,3> m_u; // right vector 
   CVector<float,3> m_v; // up vector 
   CVector<float,3> m_n; // negative of heading vector / to vector
};

#endif