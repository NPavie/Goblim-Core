#version 430

uniform sampler2D firstFragment;
uniform sampler2D colorAccumulator;

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


	//col.rgb = col.w > 1.0 ? (col.rgb / col.w) : col.rgb ;
	col.rgb = col.w > 0.01 ? (col.rgb / col.w) : col.rgb ;
	col.w = col.w > 1.0 ? 1.0 : col.w;
	//col.rgb *= col.w;
	
	if(useBestFragment && useSplatting)
	{
		bestColor.rgb = mix(col.rgb,bestColor.rgb,bestColor.a);
		Color = vec4(bestColor.rgb,col.w );
	}

	if(!useBestFragment && useSplatting)
	{
		Color = col;
	}

	if(useBestFragment && !useSplatting)
	{
		Color = bestColor;
	}
	
}