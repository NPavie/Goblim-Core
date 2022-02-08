#version 430 core
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#line 3 




layout(std140) uniform CPU
{	
	mat4 NormalMV;
	bool use_normal;
	mat4 modelMatrix;
};

in vec3 texCoord;
in vec3 fpos;
in mat3 TSpace;

uniform sampler2D colorTex;
uniform sampler2D normalTex;


layout (location = 0) out vec4 Color;
void main()
{
	vec2 tCoord = texCoord.xy;


	vec4 color = vec4(1.0);
	color =  texture(colorTex,tCoord.xy);
	vec3 i_normal = vec3(0.5,0.5,1.0);
	vec4 normalSample = vec4(i_normal,1.0);
	if(use_normal == true)	normalSample = texture(normalTex,tCoord);

	vec4 normalFromTSPace = vec4(TSpace * (normalSample.xyz*2.0 - 1.0) ,0.0);
	vec3 normal = (NormalMV * normalFromTSPace).xyz;
	
	Color = addPhong(fpos,normalize(normal),color,color,vec4(1.0),vec4(0.4,0.8,0.4,64.0));
	//Color = addTriLight(fpos,normalize(camDir),normalize(normal), 1.0*vec4(1.0,0.7,0.4,1.0),0.3*vec4(0.2,0.8,0.3,1.0) ,vec4(0.1,0.1,0.4,1.0));	
	
	//Color = vec4(normalize(normal.xyz),1.0);

}