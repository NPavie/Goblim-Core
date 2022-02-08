#version 420

layout(std140) uniform CPU {
	float shellHeight;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 BorderData;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Texture_Coordinates;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

struct Vertex{
	vec3 Position;
	vec3 PositionBeforeExtrusion;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 BorderData;
};

out Vertex currentVertex;

void main()
{
	currentVertex.Position = Position + shellHeight * normalize(Normal); // extrusion of the mesh to create the shell
	currentVertex.PositionBeforeExtrusion = Position; 
	currentVertex.Normal = Normal;
	currentVertex.Texture_Coordinates = vec3(Texture_Coordinates.xy,1.0);
	currentVertex.BorderData = BorderData;
	gl_Position = vec4(currentVertex.Position,1.0);	// projection on screen must be done after the shell completion
	
}