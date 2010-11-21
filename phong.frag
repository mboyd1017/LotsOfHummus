#version 120

uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

varying vec3 fvVaryingViewDir;
varying vec3 fvVaryingLightDir;
varying vec3 fvVaryingNormal;

void main( void )
{
	// compute dot product between light direction and normal
	vec3  fvLightDir = normalize( fvVaryingLightDir );
	vec3  fvNormal   = normalize( fvVaryingNormal );
	float fNDotL     = max(0.0, dot( fvNormal, fvLightDir )); 
   
	// compute dot product between reflection direction and view direction
	vec3  fvReflec   = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDir ); 
	vec3  fvViewDir  = normalize( fvVaryingViewDir );
	float fRDotV     = max( 0.0, dot( fvReflec, fvViewDir ) );
   
    // compute ambient, diffuse and specular colors
	vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 
	vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );

	// compute final fragment color
	gl_FragColor = fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular;
}