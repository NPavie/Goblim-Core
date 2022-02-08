#version 420

//layout(std140) uniform CPU
//{	
//};

uniform sampler2D fboIn;
uniform sampler2D fboIn1;
uniform sampler2D fboIn2;
uniform sampler2D fboIn3;
in vec3 texCoord;
layout (location = 0) out vec4 Color;
void main()
{

float m = 1;
 vec4 sum = vec4(0.0);
 
// blur in y (vertical)
// take nine samples, with the distance blurSize between them

   sum += texture(fboIn, texCoord.xy);
   sum += texture(fboIn1, texCoord.xy);
  sum += texture(fboIn2, texCoord.xy);
   sum += texture(fboIn3, texCoord.xy);
	Color =  sum;

}


/*
#version 420

//layout(std140) uniform CPU
//{	
//};

uniform sampler2D fboIn;
in vec3 texCoord;
layout (location = 0) out vec4 Color;
void main()
{

float m = 1;

vec4 colBase = texture(fboIn,texCoord.xy);
const float blurSize = 1.0/512.0;
 vec4 sum = vec4(0.0);
 
// blur in y (vertical)
// take nine samples, with the distance blurSize between them

   sum += 0.25*textureLod(fboIn, texCoord.xy,0.0);
   sum += 0.25*textureLod(fboIn, texCoord.xy,1.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,2.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,3.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,4.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,5.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,6.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,7.0);
   sum += 0.95*textureLod(fboIn, texCoord.xy,8.0);
	Color =  sum;

}
*/