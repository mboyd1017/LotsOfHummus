#version 120

uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

// TODO: Part 3
// uncomment the following two lines 
// to make the handles to the color and 
// the normal map available
uniform sampler2D colorMap;
uniform sampler2D normalMap;

varying vec3 fvVaryingViewDir;
varying vec3 fvVaryingLightDir;
// TODO: part 3
// uncomment the varying normal variable
// we don't need it anymore, because we 
// get the normal information from the normal 
// map instead
varying vec3 fvVaryingNormal;

// TODO: Part 2
// You defined an interpolating variable for the texture coordinates
// in the vertex shader. Declare the same variable here to make sure 
// you can access it. 

uniform sampler2D textureMapHandle;
varying vec2 interpolatedTextureMapCoordinates;

// Note: Applying transformations to uniforms in the vertex 
// shader is not very efficient because it is executed 
// per vertex instead of just once. However, this code 
// helps you to understand how transformations are 
// handled in the GPU and you'll get some more 
// practice with shader programming.

void main( void )
{
	// TODO: Part 3
	// fetch the normal encoded as rgb color from the normal map and
	// uncrompress it
	// The handle to the 2D normal map texture is "normalMap"
	vec4 normColor = texture2D( normalMap, interpolatedTextureMapCoordinates ); 
	vec3 fvNormal;
	fvNormal.x = 2.0*(normColor.x - 0.5f);
	fvNormal.y = 2.0*(normColor.y - 0.5f);
	fvNormal.z = 2.0*(normColor.z - 0.5f);
	
	// compute dot product between light direction and normal
	vec3  fvLightDir = normalize( fvVaryingLightDir );
	// TODO: Part 3
	// we don't need the following line of code anymore, because 
	// we get the normal information from the normal map instead 
	//vec3  fvNormal         = normalize( fvVaryingNormal );
	float fNDotL           = max(0.0, dot( fvNormal, fvLightDir )); 
   
	// compute dot product between reflection direction and view direction
	vec3  fvReflec     = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDir ); 
	vec3  fvViewDir  = normalize( fvVaryingViewDir );
	float fRDotV           = max( 0.0, dot( fvReflec, fvViewDir ) );
   
    // compute ambient, diffuse and specular colors
	vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
	vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

	// TODO: Part 2
	// Get the texel information at the interpolated texture coordinates here.
    // The handle to the 2D color map texture is "colorMap".
	vec4 texColor = texture2D( colorMap, interpolatedTextureMapCoordinates ); 

	// compute final fragment color
	// TODO: Part 2
	// Add the texture contribution to the final fragment value.
	gl_FragColor = texColor*(fvTotalAmbient + fvTotalDiffuse) + fvTotalSpecular;
}