#version 120

attribute vec3 fvNormal;		// in object coordinates

uniform vec3 fvLightWorldCoords;	// in world coordinates

uniform mat4 fmProj;	// projection matrix
uniform mat4 fmView;	// view matrix
uniform mat4 fmModel;	// model matrix

varying vec3 fvVaryingViewDir;
varying vec3 fvVaryingLightDir;
varying vec3 fvVaryingNormal;

// Applying transformations to uniforms in the vertex 
// shader is not very efficient because it is executed 
// per vertex instead of just once. However, this code 
// helps you to understand how transformations are 
// handled in the GPU and you'll get some more 
// practice with shader programming.   

void main( void )
{
	// extract rotation and translation of view matrix
	mat3 fmViewRot;
	fmViewRot[0] = fmView[0].xyz;
	fmViewRot[1] = fmView[1].xyz;
	fmViewRot[2] = fmView[2].xyz; 
	vec3 fmViewTrans = fmView[3].xyz;
	
	// compute eye position / camera center in world coordinates
	vec3 fvEyeWorldCoords = -transpose(fmViewRot) * fmViewTrans;

	// extract rotation and translation of model matrix
	mat3 fmModelRot;
	fmModelRot[0] = fmModel[0].xyz;
	fmModelRot[1] = fmModel[1].xyz;
	fmModelRot[2] = fmModel[2].xyz; 
	vec3 fmModelTrans = fmModel[3].xyz;

	// transform vertex to world coordinates
	vec4 fvVertexWorldCoords = fmModel * gl_Vertex;

	// transform normal to world coordinates
	vec3 fvNormalWorldCoords = fmModelRot * fvNormal;
	
	// viewing direction
	fvVaryingViewDir = fvEyeWorldCoords - fvVertexWorldCoords.xyz;		// in world coordinates
	
	// light direction
	fvVaryingLightDir = fvLightWorldCoords - fvVertexWorldCoords.xyz;	// in world coordinates
	
	// normal
	fvVaryingNormal = fvNormalWorldCoords;								// in world coordinates
	
	// transform vertex
	gl_Position = fmProj * fmView * fmModel * gl_Vertex;
}