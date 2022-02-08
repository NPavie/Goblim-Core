#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator
#include "/Materials/Common/Grid"						// Outil de subdivision de l'espace
#include "/Materials/Common/Lighting/Lighting"			// Modèles d'illumination								
#line 7

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif

layout(location=0) out vec4 Color;

in vec4 v_position;

void main()
{
	Color= vec4(1.0,0.0,0.0,1.0);
}