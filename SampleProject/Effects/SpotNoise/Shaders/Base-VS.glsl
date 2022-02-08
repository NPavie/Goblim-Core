#version 420

out gl_PerVertex {
	vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

layout (location = 0) in vec3 Position;

out vec3 texCoord;

void main()
{
	gl_Position = vec4(Position,1.0);
	texCoord = 0.5*(Position.xyz+1.0);

}
