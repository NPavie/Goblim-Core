#version 420

layout(std140) uniform CPU {
	mat4 objectToScreen;					/// Model view projection matrix
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

out Vertex currentSample;

void main()
{
	currentSample.Position = Position;
	currentSample.PositionBeforeExtrusion = Position; // No extrusion Done
	currentSample.Normal = Normal;
	currentSample.Texture_Coordinates = vec3(Texture_Coordinates.xy,0.0);
	currentSample.BorderData = BorderData;
	gl_Position = objectToScreen * vec4(currentSample.Position,1.0);
	
}