#version 120

attribute vec3 fvNormal;		// in object coordinates

// TODO: Part 2
// add a texture mapping attribute
attribute vec2 fvTexCoords;

// TODO: Part 3
// add attributes for the tangent and the 
// binormal
attribute vec3 fvTangent;
attribute vec3 fvBinormal;

uniform vec3 fvLightWorldCoords;	// in world coordinates

uniform mat4 fmProj;	// projection matrix
uniform mat4 fmView;	// view matrix
uniform mat4 fmModel;	// model matrix

varying vec3 fvVaryingViewDir;	// viewing direction
varying vec3 fvVaryingLightDir; // light direction
// TODO: Part 3
// comment the interpolating variable for the normal 
// we don't need this variable anymore, because we 
// get the normal information from the normal map instead
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
	// TODO: Part 3
	// we don't need the normal information anymore, 
	// because we are getting it from the normal map 
	// texture in the fragment shader
	vec3 fvNormalWorldCoords = fmModelRot * fvNormal;
	//fvVaryingNormal = fvNormalWorldCoords;								// in world coordinates
	fvVaryingNormal = fvNormal;											// object coords (not that we care)
	
	// TODO: Part 3
	// construct the [Tangent, Binormal, Normal] matrix
	mat3 RotTMatrix;
	RotTMatrix[0] = fvTangent;
	RotTMatrix[1] = fvBinormal;
	RotTMatrix[2] = fvNormal;
		
	// compute viewing direction
	// TODO: Part 3
	// transform the computed viewing direction to tangent space
	//fvVaryingViewDir = fvEyeWorldCoords - fvVertexWorldCoords.xyz;		// in world coordinates
	vec3 fvViewObjDir = fvEyeObjCoords - fvVertexObjCoords.xyz;			// obj coords
	fvVaryingViewDir = transpose(RotTMatrix)*fvViewObjDir;				// tangent space
	
	// compute lighting direction
	// TODO: Part 3
	// transform the computed lighting direction to tangent space
	// fvVaryingLightDir = fvLightWorldCoords - fvVertexWorldCoords.xyz;	// in world coordinates
	vec3 fvLightObjDir = fvLightObjCoords - fvVertexObjCoords.xyz;			// obj coords
	fvVaryingLightDir = transpose(RotTMatrix)*fvLightObjDir;				// tangent space
	
	// TODO: Part 2
	// assign the texture coordinates to an interpolating variable
	interpolatedTextureMapCoordinates = fvTexCoords;

	// transform vertex
	gl_Position = fmProj * fmView * fmModel * gl_Vertex;

}