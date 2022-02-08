/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2DArray GBuffers;
uniform sampler2D DepthSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
    float VStep;
};

layout (location = 0) out float Edge;

vec2 Offsets[8] = vec2[]
(
    vec2(-1.0 * VStep,  1.0 * HStep),
    vec2( 0.0 * VStep,  1.0 * HStep),
    vec2( 1.0 * VStep,  1.0 * HStep),
    vec2( 1.0 * VStep,  0.0 * HStep),
    vec2( 1.0 * VStep, -1.0 * HStep),
    vec2( 0.0 * VStep, -1.0 * HStep),
    vec2(-1.0 * VStep, -1.0 * HStep),
    vec2(-1.0 * VStep,  0.0 * HStep)
);

void main()
{
	vec4 Dots1, Dots2;
	vec4 Depths1, Depths2;
	vec2 offsetCoords[8];

    for(int i = 0; i < 8; i++)
    {  
        offsetCoords[i] = texCoord + Offsets[i];
    }
	
	//----- Depth -----//
	vec4 Depth = texture(DepthSampler,texCoord);
	
	Depths1.x = texture(DepthSampler, offsetCoords[0]).r;
    Depths1.y = texture(DepthSampler, offsetCoords[1]).r;
    Depths1.z = texture(DepthSampler, offsetCoords[2]).r;
    Depths1.w = texture(DepthSampler, offsetCoords[3]).r;
    Depths2.x = texture(DepthSampler, offsetCoords[4]).r;
    Depths2.y = texture(DepthSampler, offsetCoords[5]).r;
    Depths2.z = texture(DepthSampler, offsetCoords[6]).r;
    Depths2.w = texture(DepthSampler, offsetCoords[7]).r;

	vec4 DepthDeltas1 = abs(Depths1 - Depth);
    vec4 DepthDeltas2 = abs(Depth - Depths2);
	vec4 MinDepthDeltas = max(min(DepthDeltas1, DepthDeltas2), 0.00001);
    vec4 MaxDepthDeltas = max(DepthDeltas1, DepthDeltas2);

    vec4 DepthResults = step(MinDepthDeltas * 25.0, MaxDepthDeltas);

	//----- Normal -----//
	vec4 Normal = vec4(texture(GBuffers, vec3(texCoord.xy, 5.0)).xyz, 1.0);
	 
    Dots1.x = dot(texture(GBuffers, vec3(offsetCoords[0], 5.0)), Normal);         
    Dots1.y = dot(texture(GBuffers, vec3(offsetCoords[1], 5.0)), Normal);
    Dots1.z = dot(texture(GBuffers, vec3(offsetCoords[2], 5.0)), Normal);
    Dots1.w = dot(texture(GBuffers, vec3(offsetCoords[3], 5.0)), Normal);
    Dots2.x = dot(texture(GBuffers, vec3(offsetCoords[4], 5.0)), Normal);
    Dots2.y = dot(texture(GBuffers, vec3(offsetCoords[5], 5.0)), Normal);
    Dots2.z = dot(texture(GBuffers, vec3(offsetCoords[6], 5.0)), Normal);
    Dots2.w = dot(texture(GBuffers, vec3(offsetCoords[7], 5.0)), Normal);

    vec4 DotDeltas = abs(Dots1 - Dots2);

    vec4 NormalResults = step(0.4, DotDeltas);

	//----- AntiAliassing -----//	
    vec4 Results = max(NormalResults, DepthResults);

    float EdgeWeight = (Results.x + Results.y + Results.z + Results.w) * 0.25;

	if(EdgeWeight > 0.0)
		Edge = (1.0);
	else
		Edge = (0.0);
}