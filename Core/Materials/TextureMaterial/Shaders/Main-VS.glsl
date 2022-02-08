#version 420

layout(std140) uniform CPU
{
	mat4 MVP;
	mat4 MV;
};
 out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
    };

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 TexCoord;
layout (location = 4) in vec4 Tangent;



out vec3 texCoord;
out vec3 fpos;
out mat3 TSpace;

void main()
{
	gl_Position = MVP * vec4(Position,1.0);
 	fpos = (MV * vec4(Position,1.0)).xyz;
 	
	texCoord = TexCoord;

	vec3 normal = (MV * vec4(Normal,0.0)).xyz;
	vec3 B = normalize(Tangent.w*(cross(Normal,Tangent.xyz)));
	TSpace = mat3(Tangent.xyz,B,Normal);

}
