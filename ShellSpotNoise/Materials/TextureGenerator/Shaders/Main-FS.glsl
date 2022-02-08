#version 430


/**
*	@author Nicolas Pavie, Nicolas.pavie@xlim.fr
*	@date	October 25, 2012
*	@version 1.0
*/

in Vertex{
	vec3 Position;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 Tangent;
}v_in;


// test : multiple output for FBO rendering
layout (location = 0) out vec4 ColorPos;
layout (location = 1) out vec4 ColorNor;
layout (location = 2) out vec4 ColorTan;
layout (location = 3) out vec4 ColorBi;
// */


//out vec4 Color;

/**
*	@brief unmapping the object Tangent Space data to screen (MRT : Position / Normal / Tangent / Bitangent)
*	@return RGB : data, A : 1.0 if value exists, else 0
*/
void main(){
	
	// Full tangent space precomputation per surface point
	ColorPos = vec4(v_in.Position,1.0);
	ColorNor = vec4(v_in.Normal,1.0);
	ColorTan = vec4(v_in.Tangent,1.0);

	vec3 bitangent = normalize(cross(v_in.Normal,v_in.Tangent));
	ColorBi = vec4(bitangent,1.0);
	
	// */
}