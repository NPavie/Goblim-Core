#version 420

// Pipeline principale : passage du vertex au fragment directement

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Texture_Coordinates;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


layout(std140) uniform CPU 
{
	mat4 m44_Object_toScreen;
	float f_Grid_height;
	
};



struct Vertex{
	vec4 Position;
	vec4 Normal;
	vec4 Texture_Coordinates;
};

out Vertex _vertex;



void main()
{
	// rajout de l'extrusion
	//_vertex.Position = vec4(Position + f_Grid_height * normalize(Normal),1.0) ;
	_vertex.Position = vec4(Position,1.0) ;
	_vertex.Normal = vec4(normalize(Normal),1.0);
	_vertex.Texture_Coordinates = vec4(Texture_Coordinates,1.0);
	
	gl_Position = m44_Object_toScreen * _vertex.Position;
	
}