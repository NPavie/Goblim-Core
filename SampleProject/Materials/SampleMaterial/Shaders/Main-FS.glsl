#version 430

in Vertex{
	vec3 Position;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 Tangent;
}v_in;



layout (location = 0) out vec4 Color;

// For multiple render target
//layout (location = 1) out vec4 Color1;
//layout (location = 2) out vec4 Color2;
//layout (location = 3) out vec4 Color3;
// */

void main()
{
	Color = vec4(1.0,0.0,0.0,1.0);
	//Color1 = /* Whatever */ ;
	//Color2 = /* Whatever */ ;
	//Color3 = /* Whatever */ ;

	
	// */
}