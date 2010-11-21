#version 130

uniform samplerCube cubeMap;

varying vec3 fvVaryingTexCoords;

void main( void )
{  
   gl_FragColor = texture( cubeMap, fvVaryingTexCoords );
}