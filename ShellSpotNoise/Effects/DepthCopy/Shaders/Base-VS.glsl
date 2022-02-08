#version 430

layout(std140) uniform CPU
{
	vec4 box;
};
 out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
    };
layout (location = 0) in vec3 Position;

out vec3 texCoord;
void main()
{
	texCoord = 0.5*(Position.xyz+1.0);
	texCoord.y = 1.0 - texCoord.y;
	vec2 pos;
	pos.xy = box.xy - 1.0 + max(vec2(0.0),Position.xy)*box.zw;
	pos.y *= -1.0;
	gl_Position = vec4(pos,1.0,1.0);


}
