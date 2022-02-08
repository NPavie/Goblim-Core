#version 430


uniform sampler2D resourceSampler;

in vec3 texCoord;

void main()
{

	gl_FragDepth = texture(resourceSampler, texCoord.xy).r;	

}