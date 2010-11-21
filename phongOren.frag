uniform vec4 fvAmbient;
uniform vec4 fvSpecular;
uniform vec4 fvDiffuse;
uniform float fSpecularPower;
uniform vec4 fvBaseColor;

varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;
varying vec4 Color;

float mb_OrenN(vec3, vec3, vec3);

void main( void )
{
   vec3  fvLightDirection = normalize( LightDirection );
   vec3  fvNormal         = normalize( Normal );
   float fNDotL           = max(0.0, dot( fvNormal, fvLightDirection )); 
   vec3  fvViewDirection  = normalize( ViewDirection );

   float orenResult = mb_OrenN(fvLightDirection, fvNormal, fvViewDirection);
   
   vec3  fvReflection     = normalize( ( ( 2.0 * fNDotL ) * fvNormal ) - fvLightDirection ); 
  
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );
   
   vec4  fvTotalAmbient   = fvAmbient * fvBaseColor; 

   //vec4  fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	vec4 fvTotalDiffuse = fvDiffuse * orenResult * fvBaseColor;

   vec4  fvTotalSpecular  = fvSpecular * ( pow( fRDotV, fSpecularPower ) );
  
   gl_FragColor = ( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular );
 }

 float mb_OrenN( vec3 I, vec3 N, vec3 V)
{
	float rough = 0.5;
   float cosL = dot(I, N);
   float cosV = dot(V, N);
   float cosPhiDiff = dot(I-dot(N, I)*N, V - dot(N, V)*N);
   float oA = 1.0 - (0.5)*(rough*rough/(rough*rough+0.33));
   float oB = (0.45)*(rough*rough)/(rough*rough + 0.09);

   // Lookout for theta being outside 0/180
   float alpha = max(acos(cosL), acos(cosV));
   float beta = min(acos(cosL), acos(cosV));
   float L = max(0.0, cosL)*(oA+oB*max(0.0, cosPhiDiff)*sin(alpha)*tan(beta)); 
   return L;
}