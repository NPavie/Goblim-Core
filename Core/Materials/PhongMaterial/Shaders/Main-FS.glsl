#version 420

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#line 6 


layout(std140) uniform CPU
{	
mat4 NormalMV;
vec3 globalColor;
};

in vec3 texCoord;
in vec3 camDir;
in mat3 TSpace;
in vec3 fpos;
in vec3 normal;

in vec3 lDir;
in vec3 cDir;

in vec3 v_Normal;
in vec4 v_Tangent;

layout (location = 0) out vec4 Color;
void main()
{

vec3 B = normalize(v_Tangent.w*(cross(normalize(v_Normal),normalize(v_Tangent.xyz))));
mat3 FTSpace = transpose(mat3(normalize(v_Tangent.xyz),B,normalize(v_Normal)));


vec4 color =  vec4(globalColor,1.0);
//vec3 normal = (NormalMV * vec4(transpose(FTSpace) * vec3(0.0,0.0,1.0),0.0)).xyz;//normalize(2.0*texture(normalTex,texCoord.xy).xyz-1.0).xyz,0.0)).xyz;
vec3 normal = normalize((NormalMV * vec4(v_Normal,0.0)).xyz);
//normal = vec3(0.0,0.0,1.0);

	Color = addPhong(fpos,normalize(camDir),normalize(normal),color,color,vec4(1.0),vec4(0.1,0.8,0.3,128.0));

	//Color = addTriLight(fpos,normalize(camDir),normalize(normal), 1.0*vec4(1.0,0.7,0.4,1.0),0.3*vec4(0.2,0.8,0.3,1.0) ,vec4(0.1,0.1,0.4,1.0));
	//Color = Color*color;
	//color = vec4(0.5,0.5,0.5,1.0);
	//Color = addTriLight(fpos,normalize(camDir),normalize(normal), color*vec4(1.0,0.8,0.6,1.0),color*0.4*vec4(0.31,0.11,0.0,1.0) , color*0.7*vec4(0.31,0.11,0.0,1.0));

}