#version 420

layout(std140) uniform CPU_VS
{
	mat4 MVP;
	mat4 MV;
	vec3 lightColor_;
	vec3 lightPos;
	vec3 camPos;

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
out vec3 camDir;
out mat3 TSpace;
out vec3 normal;

out vec3 v_Normal;
out vec4 v_Tangent;

out vec3 lDir;
out vec3 cDir;
void main()
{
gl_Position = MVP * vec4(Position,1.0);
 fpos = (MV * vec4(Position,1.0)).xyz;
 normal = (MV * vec4(Normal,0.0)).xyz;
texCoord = TexCoord;

vec3 B = normalize(Tangent.w*(cross(Normal,Tangent.xyz)));
TSpace = transpose(mat3(Tangent.xyz,B,Normal));

v_Normal = Normal;
v_Tangent = Tangent;

camDir = normalize(camPos - fpos);

lDir = lightPos - Position;

cDir =    TSpace * normalize(camPos - Position);
lDir =   TSpace * normalize(lDir);


}
