#version 420

layout(std140) uniform CPU
{	
	float texSize;
};

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

uniform sampler2D fboIn;
in vec3 texCoord;
layout (location = 0) out vec4 Color;
void main(void)
{
    Color = texture( fboIn, vec2(texCoord) ) * weight[0];
    for (int i=1; i<3; i++) {
        Color += texture( fboIn,texCoord.xy+vec2( offset[i]/texSize,0.0)) * weight[i];
        Color += texture( fboIn,texCoord.xy-vec2(offset[i]/texSize,0.0)) * weight[i];
    }

}
