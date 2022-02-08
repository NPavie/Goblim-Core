#version 440 core




uniform sampler2D ColorSampler;
uniform sampler2D DepthSampler;

in vec2 texCoord;

layout (location = 0) out vec3 Color;

layout(std140) uniform CPU {
	float time;
};

vec3 grayScale(vec3 c) {
	return vec3(c.r*0.2126 + c.g*0.7152 + c.b*0.0722);
}


void main()
{
	float PI = 3.1415927410125732421875;
	float teta = 0.0;

	vec2 ruv = texCoord;
 
    ruv -= 0.5;
    ruv *= 1.1;
    float d = length(ruv.xy);
    const float max_radius = 0.70711;
    float modified = sin((d/max_radius/2.5)*PI)/1.55;
    ruv += ruv*(d-modified);
    
    ruv*=(2.2-1.1);
 
    
    ruv += 0.5;

	vec3 col = vec3(0.04);

	if((ruv.x>0)&&(ruv.y>0)&&(ruv.x<1)&&(ruv.y<1)) {
		vec3 c = texture( ColorSampler, ruv ).rgb;
		//vec3 c = texture(DepthSampler, texCoord).rrr;
			
		col = (c);

		float interlaceFrequency = 230;
		float interlaceIntensity = 1.0;
		float interlace = 1+clamp( sin( ruv.y*PI*interlaceFrequency )*0.25, 0.0, 0.25 ) * interlaceIntensity;

		float flicks = 1;
		flicks += sin((ruv.y+time*5)*PI*0.99)*0.12;
		flicks += sin((ruv.y+time)*PI*0.12)*0.12;


		col *= interlace * flicks* vec3(0.64, 0.64, 1.0);
	}
	else 
		col = ( texture( ColorSampler, texCoord ).rgb )*0.18* vec3( 0.64, 0.64, 1.0 ) ;
		//col = ( texture(DepthSampler, texCoord).rrr )*0.18* vec3( 0.64, 0.64, 1.0 ) ;

	/*float thickness = 0.001;
	if( ( ( ruv.x>=0.0 ) && ( ruv.x<=thickness ) && ( ruv.y>=0.0 ) && ( ruv.y<=1.0 ) ) 
	|| ( ( ruv.x<=1.0 ) && ( ruv.x>=1.0-thickness ) && ( ruv.y>=0.0 ) && ( ruv.y<=1.0 ) ) 
	|| ( ( ruv.y>=0.0 ) && ( ruv.y<=thickness ) && ( ruv.x>=0.0 ) && ( ruv.x<=1.0 ) ) 
	|| ( ( ruv.y<=1.0 ) && ( ruv.y>=1.0-thickness ) && ( ruv.x>=0.0 ) && ( ruv.x<=1.0 ) ) )
		col = vec3(0.12);*/

	Color = col;
}