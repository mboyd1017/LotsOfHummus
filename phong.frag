uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;
varying vec4 Color;

void main( void )
{
   vec3  fvLightDirection = normalize( LightDirection );
   vec3  fvNormal         = normalize( Normal );
   float fNDotL           = max(0.0, dot( fvNormal, fvLightDirection )); 
   
   vec3  fvReflection     = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDirection ); 
   vec3  fvViewDirection  = normalize( ViewDirection );
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );
   
   vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 

   vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
 
   vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );
  
   gl_FragColor = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular );
 }