#version 120

attribute vec3 fvNormal;		// in object coordinates

// TODO: Part 2
// add a texture mapping attribute
attribute vec2 fvTexCoords;

uniform vec3 fvLightWorldCoords;	// in world coordinates

uniform mat4 fmProj;	// projection matrix
uniform mat4 fmView;	// view matrix
uniform mat4 fmModel;	// model matrix

varying vec3 fvVaryingViewDir;	// viewing direction
varying vec3 fvVaryingLightDir; // light direction
varying vec3 fvVaryingNormal;	// normal

// TODO: Part 2
// As for the normal, the viewing and the light directions, we need an 
// interpolating texture coordinate variable that is handed over to 
// the fragment shader. 
varying vec2 interpolatedTextureMapCoordinates;

// Note: Applying transformations to uniforms in the vertex 
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

	// TODO: Part 2
	// transform eye position / camera center to object coordinates
	vec3 fvEyeObjCoords = transpose(fmModelRot)*(fvEyeWorldCoords - fmModelTrans);

	// TODO: Part 2
	// transform light source position to object coordinates
	vec3 fvLightObjCoords = transpose(fmModelRot)*(fvLightWorldCoords - fmModelTrans); 

	// transform vertex to world coordinates
	// TODO: Part 2
	// use the vertex position in object coordinates instead
	vec4 fvVertexWorldCoords = fmModel * gl_Vertex;
	vec3 fvVertexObjCoordsXYZ = transpose(fmModelRot)*(fvVertexWorldCoords.xyz - fmModelTrans);
	/* There must be a more graceful way to do this */
	vec4 fvVertexObjCoords;
	fvVertexObjCoords.x = fvVertexObjCoordsXYZ.x;
	fvVertexObjCoords.y = fvVertexObjCoordsXYZ.y;
	fvVertexObjCoords.z = fvVertexObjCoordsXYZ.z;
	fvVertexObjCoords.w = 1.0;

	// transform normal to world coordinates
	// TODO: Part 2
	// use the normal in object coordinates instead+
	vec3 fvNormalWorldCoords = fmModelRot * fvNormal;

	// fvVaryingNormal = fvNormalWorldCoords;								// in world coordinates
	fvVaryingNormal = fvNormal;										// object coordinates

	// compute viewing direction
	// TODO: Part 2
	// compute the viewing direction in object coordinates instead
	// fvVaryingViewDir = fvEyeWorldCoords - fvVertexWorldCoords.xyz;		// in world coordinates
	fvVaryingViewDir = fvEyeObjCoords - fvVertexObjCoords.xyz;		// object coordinates
	
	// compute lighting direction
	// TODO: Part 2
	// compute the light direction in object coordinates instead
	// fvVaryingLightDir = fvLightWorldCoords - fvVertexWorldCoords.xyz;	// in world coordinates
	fvVaryingLightDir = fvLightObjCoords - fvVertexObjCoords.xyz;
	
	// TODO: Part 2
	// assign the texture coordinates to an interpolating variable
	interpolatedTextureMapCoordinates = fvTexCoords;

	// transform vertex
	gl_Position = fmProj * fmView * fmModel * gl_Vertex;

}