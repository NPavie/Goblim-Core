#version 430

uniform sampler2D firstFragment;
uniform sampler2D colorAccumulator;

in vec3 texCoord;

layout (location = 0) out vec4 Color;

layout(std140) uniform CPU
{
	bool useBestFragment;
};


void main()
{
	vec4 col = textureLod(colorAccumulator, texCoord.xy,0);
	col.rgb = col.w > 1.0 ? (col.rgb / col.w) : col.rgb ;
	col.w = col.w > 1.0 ? 1.0 : col.w;
	//col.rgb *= col.w;
	
	if(useBestFragment)
	{

		vec4 bestColor = textureLod(firstFragment, texCoord.xy,0); 
		bestColor.rgb = mix(col.rgb,bestColor.rgb,bestColor.a);
		Color = vec4(bestColor.rgb,col.w );
	}
	else
	{
		Color = col;
	}
}