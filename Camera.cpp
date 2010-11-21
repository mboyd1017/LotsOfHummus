#include "Camera.h"

#include <cmath>

Camera::Camera( float fov, float aspectRatio, float nearPlane, float farPlane )
   : 
   m_fov(fov),
   m_aspectRatio(aspectRatio),
   m_nearPlane(nearPlane), 
   m_farPlane(farPlane)
{
   m_eye = CVector<float,3>(0.0f, 0.0f, 0.0f);
   m_u = CVector<float,3>(1.0f, 0.0f, 0.0f);
   m_v = CVector<float,3>(0.0f, 1.0f, 0.0f);
   m_n = CVector<float,3>(0.0f, 0.0f, 1.0f);
}

void Camera::set(	float eyeX, float eyeY, float eyeZ, 
						float lookX, float lookY, float lookZ, 
						float upX, float upY, float upZ )
{
   m_eye = CVector<float,3>( eyeX, eyeY, eyeZ );
	m_n = m_eye - CVector<float,3>( lookX, lookY, lookZ );
	m_v = CVector<float,3>( upX, upY, upZ );
	m_u = vecCross( CVector<float,3>( upX, upY, upZ ), m_n );
   m_n = m_n.normalize( );
	m_u = m_u.normalize( );
	m_v = m_v.normalize( );
}

void Camera::setAspectRatio( float aspectRatio )
{
	m_aspectRatio = aspectRatio;
}

void Camera::setFov( float fov )
{
	m_fov = fov;
}

void Camera::setNearPlane( float nearPlane )
{
	m_nearPlane = nearPlane;
}

void Camera::setFarPlane( float farPlane )
{
	m_farPlane = farPlane;
}


void Camera::roll( float rollAngle )
{
	m_v = m_v + (0.1f*sin(rollAngle)*m_u);
	m_v = m_v.normalize();
	m_u = vecCross(m_v, m_n);
	apply();
}

void Camera::pitch( float pitchAngle )
{
	m_v = m_v - (0.1f*(sin(pitchAngle))*m_n);
	m_v = m_v.normalize();
	m_n = vecCross(m_u, m_v);
	apply();
}

void Camera::yaw( float yawAngle )
{
	m_u = m_u - (0.1f*sin(yawAngle)*m_n);
	m_u = m_u.normalize();
	m_n = vecCross(m_u, m_v);
	apply();
}

void Camera::slide( float dU, float dV, float dN )
{
	m_eye += dU * m_u;
	m_eye += dV * m_v;
	m_eye += dN * m_n;
}

void Camera::apply( ) const
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( m_fov, m_aspectRatio, m_nearPlane, m_farPlane );

	glMatrixMode( GL_MODELVIEW );
	GLfloat m[16];
	m[0] = m_u[0]; m[4] = m_u[1]; m[8] = m_u[2];		m[12] = -vecDot(m_eye, m_u);
	m[1] = m_v[0]; m[5] = m_v[1]; m[9] = m_v[2];		m[13] = -vecDot(m_eye, m_v);
	m[2] = m_n[0]; m[6] = m_n[1]; m[10] = m_n[2];	m[14] = -vecDot(m_eye, m_n);
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;		m[15] = 1.0f;
	glLoadMatrixf(m);
}