#version 420

layout(std140) uniform CPU
{	
	float bloomTreshold;
};

uniform sampler2D fboIn;
in vec3 texCoord;
layout (location = 0) out vec4 Color;
void main()
{

vec4 colBase = texture(fboIn,texCoord.xy); 
float am = 2.0*smoothstep(0.0,1.0,(length(colBase.xyz) - bloomTreshold)/1.7);
//float am = 0.75*length(colBase);

vec3 brightColor = max (colBase.rgb - vec3 (bloomTreshold), vec3 (0.f));
float bright = dot (brightColor, vec3 (1.f));
bright = smoothstep (0.f, .5f, bright);

Color.rgb = mix (vec3 (0.f), colBase.rgb, bright).rgb;

}
