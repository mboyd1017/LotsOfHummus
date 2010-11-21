#version 120

uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

// TODO: Part 4
// uncomment the following two lines
// to make the handles to the cloud and
// cloud transparency map available
uniform sampler2D cloudMap;
uniform sampler2D cloudTransMap;

varying vec3 fvVaryingViewDir;
varying vec3 fvVaryingLightDir;
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
   
	// compute dot product between reflection direction and view direction
	vec3  fvReflec     = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDir ); 
	vec3  fvViewDir  = normalize( fvVaryingViewDir );
	float fRDotV           = max( 0.0, dot( fvReflec, fvViewDir ) );
   
    // compute ambient, diffuse and specular colors
	vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
	vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

	// TODO: Part 4
	// Get the texel information at the interpolated texture coordinates from 
	// the cloud and the transparency map. The two handles are "cloudMap" and 
	// "cloudTransMap".
	vec4 cloudTexColor = texture2D( cloudMap, interpolatedTextureMapCoordinates ); 
	vec4 cloudTransTexColor = texture2D( cloudTransMap, interpolatedTextureMapCoordinates ); 

	// TODO: Part 4
	// Assign the maximum r, g, b value to all color channels of the final 
	// texture color. Use the value from transparency map to compute the 
	// alpha value of the final texture color. Hint: Look at the transparency 
	// map image. In which parts of the image are the color values almost white 
	// (1.0)?
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 0.0);

	// compute final fragment color
	// TODO: Part 2
	// Add the texture contribution to the final fragment value.
	gl_FragColor = finalColor*(fvTotalAmbient + fvTotalDiffuse) + fvTotalSpecular;
	gl_FragColor.a = (1.0 - cloudTransTexColor.x);
}