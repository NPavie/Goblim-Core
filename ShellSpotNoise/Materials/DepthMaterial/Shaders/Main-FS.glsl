#version 430

in vec4 v_PositionInCamera;

in vec3 v_TexCoord;

layout (location = 0) out vec4 Color;

float LinearizeDepth(float zoverw,float near, float far)
{
	return (2.0 * near) / (far + near - zoverw * (far - near));	
}

void main()
{
	
	// Depth from viewspace coord in linear depth value [0,1] with nearPlane = 0.01 and farPlane = 200.0
	float depthInViewSpace = -v_PositionInCamera.z;
	depthInViewSpace = (depthInViewSpace - 0.01) / (200 - 0.01);

	//float depthInFrag = gl_FragCoord.z / gl_FragCoord.w;

	Color = vec4(depthInViewSpace, 0 ,0,1.0);
	//Color = vec4(v_TexCoord,1.0);

}