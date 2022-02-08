#version 420

layout(std140) uniform CPU
{
	float bloomAmount;	
};

layout( binding = 0) uniform sampler2D inColor;
//uniform sampler2D inBloom;
//uniform sampler2D inLightScatter;
//uniform sampler2D inSSAO;
layout( binding = 1) uniform sampler2D inDepth;
in vec3 texCoord;
layout (location = 0) out vec4 Color;


float linearize(float z)
{
float zNear = 1.0;
float zFar = 200.0;
    
    float z_n = 2.0 * z - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	return z_e;
}

void main()
{

float m = 1;

vec4 color = texture(inColor,texCoord.xy);
//vec4 bloom = texture(inBloom,texCoord.xy);
//vec4 lightScatter = texture(inLightScatter,texCoord.xy);
//vec4 ssao = texture(inSSAO,texCoord.xy);
//Color =  color + bloomAmount*bloom + 4.0*lightScatter ;
vec4 d = texture(inDepth,texCoord.xy);

Color = color;
float de = vec4(linearize(d));
if (de.x < 0.5)
	Color.x += 0.3;
//Color = ssao;

}