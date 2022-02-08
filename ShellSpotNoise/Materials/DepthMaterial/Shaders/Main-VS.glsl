#version 420

layout(std140) uniform CPU
{
	mat4 CPU_MVP;
	mat4 CPU_ModelView;
};

out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
};

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Texture_Coordinates;

out vec4 v_PositionInCamera;
out vec3 v_TexCoord;

void main()
{
	// Pour rasterization
	gl_Position = CPU_MVP * vec4(Position,1.0);

	// Pour le calcul du vertex dans l'espace camera
	v_PositionInCamera = (CPU_ModelView * vec4(Position,1.0)); 
	v_TexCoord = Texture_Coordinates;
	
}
