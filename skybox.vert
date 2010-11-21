#version 130

uniform mat4 fmProj;
uniform mat4 fmView;

varying vec3 fvVaryingTexCoords;
   
void main( void )
{
	// extract rotation of view matrix
	mat3 fmViewRot;
	fmViewRot[0] = fmView[0].xyz;
	fmViewRot[1] = fmView[1].xyz;
	fmViewRot[2] = fmView[2].xyz; 
	
	// TODO: Part 5
	// use the normalized vertex position in object coordinates 
	// as the three texture coordinates. 
	fvVaryingTexCoords = normalize(gl_Vertex.xyz);

	// TODO: Part 5
	// Apply the rotation of the view matrix only. 
	// We never want to go to the edge of the skybox when 
	// moving around in our planetary system.
	// To achieve this, don't apply any transformations before 
	// drawing the skybox to make sure that the object and 
	// world coordinate systems coincide.
	// If we then only use the rotation of the view matrix, 
	// but not its translation, we rotate the skybox, but the 
	// skyboxes center stays always at the camera center. 
	vec4 fvVertexViewRot;
	fvVertexViewRot.xyz = fmViewRot*gl_Vertex.xyz;
	fvVertexViewRot.w = 1.0;

	gl_Position = fmProj * fvVertexViewRot;
}