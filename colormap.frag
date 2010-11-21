#version 120

uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

// TODO: Part 2
// uncomment the following line
// it provides you with a handle 
// to the color map
uniform sampler2D colorMap;

varying vec3 fvVaryingViewDir;
varying vec3 fvVaryingLightDir;
varying vec3 fvVaryingHeadlightDir;
varying vec3 fvVaryingNormal;

// TODO: Part 2
// You defined an interpolating variable for the texture coordinates
// in the vertex shader. Declare the same variable here to make sure 
// you can access it. 

uniform sampler2D textureMapHandle;
varying vec2 interpolatedTextureMapCoordinates;

void main( void )
{
	// compute dot product between light direction and normal
	vec3  fvLightDir = normalize( fvVaryingLightDir );
	vec3  fvNormal         = normalize( fvVaryingNormal );
	float fNDotL           = max(0.0, dot( fvNormal, fvLightDir )); 


	vec3 fvHeadlightDir = normalize( fvVaryingHeadlightDir );
	float fNDotHL = max(0.0, dot( fvNormal, fvHeadlightDir ));
	if (fNDotHL < .3 || fNDotHL > 0.4) { fNDotHL = 0; }
	else fNDotHL = .6;
   
	// compute dot product between reflection direction and view direction
	vec3  fvReflec     = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDir ); 
	vec3  fvViewDir  = normalize( fvVaryingViewDir );
	float fRDotV           = max( 0.0, dot( fvReflec, fvViewDir ) );
   
    // compute ambient, diffuse and specular colors
	vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
	vec4  fvTotalDiffuse   = fvDiffuse * (fNDotL + fNDotHL)* fvBaseColor; 
	vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

	// TODO: Part 2
	// Get the texel information at the interpolated texture coordinates here.
    // The handle to the 2D color map texture is "colorMap".
	vec4 texColor = texture2D( colorMap, interpolatedTextureMapCoordinates ); 

	// compute final fragment color
	// TODO: Part 2
	// Add the texture contribution to the final fragment value.
	gl_FragColor = (texColor*(fvTotalAmbient + fvTotalDiffuse) + fvTotalSpecular) ;
}