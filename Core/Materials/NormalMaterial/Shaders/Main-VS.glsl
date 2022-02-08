#version 420

layout(std140) uniform CPU_VS
{
	mat4 MVP;
	mat4 MV;
	mat4 NMV;
	mat4 ModelMatrix;
	float near;
	float far ;

};
 out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
    };
layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;

out vec3 v_Color;
out float ldepth;

float LinearizeDepth(float zin)
{
  return (2.0 * near) / (far + near - zin * (far - near));	
}


void main()
{
gl_Position = MVP * vec4(Position,1.0);
v_Color = (NMV * vec4(normalize(Normal.xyz),0.0)).xyz;
ldepth = - (MV * vec4(Position,1.0)).z;
ldepth = (ldepth - near)/(far-near);


}