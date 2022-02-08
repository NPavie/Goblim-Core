#version 430

layout(std140) uniform CPU
{
	mat4 M;
	mat4 MV; // Model View Matrix 
	mat4 MVP; // Model View Projection Matrix
	mat4 NormalM;
	mat4 NormalMV; // Normal Model View Matrix 
	mat4 lastMVP;
};
layout (location = 0) in vec3 Position; // Position of the current vertex (3D Space)
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 TexCoord; // TexCoords from the VBO/VAO ?
layout (location = 4) in vec4 Tangent;

out gl_PerVertex 
{
	// Position of the current vertex (Screen Space)
	vec4 gl_Position; 
	// Size of rasterized point
	float gl_PointSize; 
	// Define distance for Vertex Clipping
	float gl_ClipDistance[]; 
};

out vec3 v_T;
out vec3 v_B;
out vec3 v_N;
out vec3 v_Position;
out vec3 v_TexCoord;
out mat3 v_TBN;
out vec4 v_ScreenPosition;
out vec4 v_ScreenLastPosition;
out vec4 v_PositionInCamera;

void main()
{
	//----- Object Space to Clip Space -----//
	gl_Position = MVP * vec4(Position, 1.0);

	// Nedeed for Per Object Motion Blur
	v_ScreenPosition = gl_Position;
	v_ScreenLastPosition = lastMVP * vec4(Position, 1.0);

	//----- TBN Matrix Data Computation -----//
	v_N = (Normal);
	v_N = normalize((NormalM * vec4(v_N, 0.0)).xyz);

	v_T = (Tangent.xyz / Tangent.w);
	v_T = normalize((NormalM * vec4(v_T, 0.0)).xyz);

	v_B = normalize(cross(v_N, v_T));

	v_TBN = transpose(mat3(v_T, v_B, v_N));

	//----- Transmitting Data -----//
	v_TexCoord = TexCoord;
	//vec4 tmp = (MV * vec4(Position, 1.0));
	vec4 tmp = (M * vec4(Position, 1.0));
	v_Position = tmp.xyz;
	v_Position = tmp.xyz / tmp.w;
	v_PositionInCamera = MV * vec4(Position, 1.0);

}