#version 420



layout(std140) uniform CPU
{

uniform float occluderBias;
uniform float samplingRadius;
uniform vec2 attenuation;
uniform vec2 texelSize;

};

uniform sampler2D randomMap;
uniform sampler2D normalMap;

in vec2 texCoords;
in vec3 fpos;


float occlude(vec3 pos,vec3 normal,vec2 uv)
{
	vec3 frag_pos = vec3(uv,texture(normalMap,uv).w)*2.0-1.0;
	vec3 lvec = frag_pos - pos;
	float intensity = max (dot(normalize(lvec),normal)-occluderBias,0.0);

	float dist = length(lvec);
	float att = 1.0/(attenuation.x+(attenuation.y*dist));
	return intensity*attenuation;
}

const float Sin45 = 0.707107;


 layout (location = 0) out vec4 Color;
 layout (location = 1) out vec4 Color1;
void main(void)
{
	vec2 kernel[4];
    kernel[0] = vec2(0.0, 1.0); // top
    kernel[1] = vec2(1.0, 0.0); // right
    kernel[2] = vec2(0.0, -1.0);    // bottom
    kernel[3] = vec2(-1.0, 0.0);    // left

	vec3 srcPosition = fpos;
    vec4 srcN = texture2D(normalMap, texCoords);
	vec3 srcNormal = srcN.xyz;
	float srcDepth = srcN.w;
    vec2 randVec = normalize(texture2D(randomMap, texCoords).xy * 2.0 - 1.0);

	float kernelRadius = 0.2*samplingRadius * (1.0 - srcDepth);
	float occlusion = 0.0;

	for (int i = 0; i < 4; ++i)
    {
        vec2 k1 = reflect(kernel[i], randVec);
        vec2 k2 = vec2(k1.x * Sin45 - k1.y * Sin45,
                k1.x * Sin45 + k1.y * Sin45);
       // k1 *= texelSize;
      // k2 *= texelSize;
        
        occlusion += occlude(srcPosition, srcNormal, texCoords + k1 * kernelRadius);
        occlusion += occlude(srcPosition, srcNormal, texCoords + k2 * kernelRadius * 0.75);
       occlusion += occlude(srcPosition, srcNormal, texCoords + k1 * kernelRadius * 0.5);
        occlusion += occlude(srcPosition, srcNormal, texCoords + k2 * kernelRadius * 0.25);
    }
    
    // Average and clamp ambient occlusion
    occlusion /= 16.0;
    occlusion = clamp(occlusion, 0.0, 1.0);
   // Color = vec4(srcDepth,srcDepth,srcDepth,1.0);
    //Color = vec4(randVec.xy,0.0,1.0);
	//occlusion = smoothstep(0.0,0.6,sqrt(occlusion));
	Color = vec4(occlusion,occlusion,occlusion,1.0);
	Color1 = vec4(1.0,0.0,1.0,1.0);

}