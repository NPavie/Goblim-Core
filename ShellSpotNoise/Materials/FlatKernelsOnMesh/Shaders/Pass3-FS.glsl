#version 430

uniform sampler2D firstFragment;
uniform sampler2D colorAccumulator;
uniform sampler2D revealAccumulation;

in vec3 texCoord;

layout (location = 0) out vec4 Color;

layout(std140) uniform CPU
{
	bool useBestFragment;
	bool useSplatting;
};


void main()
{
	Color = vec4(0.0);

	// Passe de recomposition
	vec4 col = textureLod(colorAccumulator, texCoord.xy,0);
	vec4 bestColor = textureLod(firstFragment, texCoord.xy,0); 

	float reveal = textureLod(revealAccumulation, texCoord.xy,0).r; 
	reveal = clamp(reveal, 0.0, 1.0);

	col.rgb = col.w > 0.01 ? (col.rgb / col.a) : col.rgb ;
	col.w = col.w >= 1.0 ? 1.0 : col.w;
	//col.rgb *= col.w;
	
	if(useBestFragment && useSplatting)
	{
		Color = vec4(mix(col.rgb,bestColor.rgb, bestColor.a), reveal);
		//Color = vec4(bestColor.rgb * bestColor.a + col.rgb * (1.0 - bestColor.a), reveal );
	}

	if(!useBestFragment && useSplatting)
	{
		Color = vec4(col.rgb, reveal );
		//if(col.w < 1.0) Color = vec4(1.0,0.0,0.0,col.w);
	}

	if(useBestFragment && !useSplatting)
	{
		Color = bestColor;
	}
	
}