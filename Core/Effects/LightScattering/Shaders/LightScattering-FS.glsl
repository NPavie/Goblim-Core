#version 420

layout(std140) uniform CPU
{
  float exposure;
  float decay;
  float density;
  float weight;
  vec2 lightPositionOnScreen;	
};

 const float NUM_SAMPLES = 50.0 ;
 uniform sampler2D lightBuffer;
 in vec3 texCoord;
 layout (location = 0) out vec4 Color;
 void main()
 {

  vec2 deltaTextCoord = vec2( texCoord.xy - lightPositionOnScreen.xy );
  vec2 textCoo = texCoord.xy;
  deltaTextCoord *= 1.0 / NUM_SAMPLES * density;
  float illuminationDecay = 1.0;

  for(float i=0; i < NUM_SAMPLES ; i++)
   {
     textCoo -= deltaTextCoord;
     vec4 lightsample = texture(lightBuffer, textCoo );
          lightsample *= illuminationDecay * 2.0*weight;
          Color += lightsample;
          illuminationDecay *= 95.0*decay;
  }
  Color *= 2.0*exposure;


}