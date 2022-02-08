
layout (std430,binding=1) readonly buffer matricesBuffer
{
	//vec4 alignTest;
	mat4 Proj;
	mat4 View;
	mat4 ViewProj;
	mat4 ViewProjInv;
	mat4 ViewProjNormal;
};
