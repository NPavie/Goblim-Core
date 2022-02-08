#version 450


/**
*	@author Nicolas Pavie, Nicolas.pavie@xlim.fr
*	@date	October 25, 2012
*	@version 1.0
*/


layout(std140) uniform FUBO 
{	
	vec4 obj_pos;
	float radius;
};




in vec3 texCoord;

uniform sampler2DArray groundData;
uniform sampler2D Previous;

// test : multiple output for FBO rendering
layout (location = 0) out vec4 bending;
// */


//out vec4 Color;

/**
*	@brief unmapping the object Tangent Space data to screen (MRT : Position / Normal / Tangent / Bitangent)
*	@return RGB : data, A : 1.0 if value exists, else 0
*/
void main(){
	
	// Full tangent space precomputation per surface point
	vec4 samplePos = textureLod(groundData,vec3(texCoord.xy,0),0);
	vec3 sampleNorm = normalize(textureLod(groundData,vec3(texCoord.xy,1),0).xyz);
	vec3 sampleTang = normalize(textureLod(groundData,vec3(texCoord.xy,2),0).xyz);
	vec3 sampleBi = normalize(textureLod(groundData,vec3(texCoord.xy,3),0).xyz);

	mat3 TBN = mat3(sampleTang,sampleBi,sampleNorm);

	


	vec4 disp = obj_pos - samplePos;

	vec3 local_disp = TBN * disp.xyz;


	float fact = 1.0 - smoothstep(0.0,1.0,length(local_disp)/radius); 

	/*
	if (length(local_disp) > radius)
		fact = 0;
	else fact = 1.0 - length(local_disp)*0.2;
*/
	bending.xyz = fact*normalize(local_disp);
	bending.w = fact;

/*
	vec4 col = textureLod(Previous,vec2(texCoord.xy),0);
	if (length(bending) < length(col))
		bending = col;
	*/
	// */
}