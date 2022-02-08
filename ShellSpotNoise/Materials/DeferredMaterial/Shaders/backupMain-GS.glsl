#version 440


layout(triangles, invocations = 3) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform CPU
{
	mat4 MV;
	mat4 MVP;
	mat4 NormalMV;
	vec3 CamPos;
};

in vec3 v_T[];
in vec3 v_B[];
in vec3 v_N[];
in vec3 v_Position[];
in vec3 v_TexCoord[];
in mat3 v_TBN[];
in vec4 v_ScreenPosition[];
in vec4 v_ScreenLastPosition[];
in vec4 v_PositionInCamera[];

in gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[3];

out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

out vec3 g_T;
out vec3 g_B;
out vec3 g_N;
out vec3 g_Position;
out vec3 g_TexCoord;
out mat3 g_TBN;
out vec4 g_ScreenPosition;
out vec4 g_ScreenLastPosition;
out vec4 g_PositionInCamera;

vec3 ComputeAveragePosition(in vec3 p0, in vec3 p1, in vec3 p2)
{
	return ((p0 + p1 + p2) / 3.0);
}

vec3 ComputeAverageNormal(in vec3 p0, in vec3 p1, in vec3 p2)
{
	return normalize(cross((p1 - p0), (p2 - p0)));
}

void main()
{
	/*// Calculate two vectors in the plane of the input triangle
	vec3 avgPos = ComputeAveragePosition(v_Position[0], v_Position[1], v_Position[2]);
	//vec3 avgNorm = ComputeAverageNormal(v_N[0], v_N[1], v_N[2]);
	vec3 avgNorm = ComputeAverageNormal(v_Position[0], v_Position[1], v_Position[2]);
 
    // Take the dot product of the normal with the view direction
	float d = dot(normalize(-avgPos), avgNorm);

	if (d >= 0.0)*/
	{
		for(int i = 0; i < 3; ++i)
		{	
			g_T			= v_T[i];
			g_B			= v_B[i];
			g_N			= v_N[i];
			g_Position	= v_Position[i];
			g_TexCoord	= v_TexCoord[i];
			g_TBN		= v_TBN[i];
			
			g_PositionInCamera = v_PositionInCamera[i];
			g_ScreenPosition = v_ScreenPosition[i];
			g_ScreenLastPosition = v_ScreenLastPosition[i];
			// Calculate the Screen Space Vertex Position
			gl_Position = gl_in[i].gl_Position;

			EmitVertex();
		}
		EndPrimitive();
	}
}