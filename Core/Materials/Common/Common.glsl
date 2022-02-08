
#ifndef COMMON_GLSL
#define COMMON_GLSL

layout (std430,binding=1) readonly buffer matricesBuffer
{
	mat4 Proj;				// Camera projection matrix (from camera to clip space)
	mat4 View;				// Camera matrix (from world to view space)
	mat4 ViewProj;			// projection * view (from world to clip space)
	mat4 ViewProjInv;		// inverse(projection * view) (may not work) (from clip space to world)
	mat4 ViewProjNormal;
};


// From http://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
// AND
// http://web.archive.org/web/20130416194336/http://olivers.posterous.com/linear-depth-in-glsl-for-real
float getEyeDepthFromDepthBuffer(
	float dephtBufferValue,
	float zNear, float zFar)
{
    float z_n = 2.0 * dephtBufferValue - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

#endif



