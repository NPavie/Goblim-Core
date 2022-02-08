#version 430

uniform sampler2D firstFragment;
uniform sampler2D colorAccumulator;

in vec3 texCoord;

layout (location = 0) out vec4 Color;

void main()
{
	// Passe de recomposition
	vec4 bestColor = textureLod(firstFragment, texCoord.xy,0); 
	vec4 col = textureLod(colorAccumulator, texCoord.xy,0);
	//col.rgb = col.w > 1.0 ? (col.rgb / col.w) : col.rgb;
	col.rgb = col.w > 1.0 ? (col.rgb / col.w) : col.rgb ;
	col.w = col.w > 1.0 ? 1.0 : col.w;
	//col.rgb *= col.w;
	
	//Color = bestColor;
	bestColor.rgb = mix(col.rgb,bestColor.rgb,bestColor.a);
	Color = vec4(bestColor.rgb,col.w );
}