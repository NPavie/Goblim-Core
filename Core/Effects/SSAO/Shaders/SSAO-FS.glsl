
#version 420


// http://www.gamedev.net/topic/556187-the-best-ssao-ive-seen/


layout(std140) uniform CPU
{
	mat4 P;
	mat4 inverseProjectionMatrix;
	float texSize;


};
uniform sampler2D randomMap;
uniform sampler2D normalMap;
uniform sampler2D colorMap;


in vec2 texCoords;
out vec4 Color;

vec3 readNormal(in vec2 coord)  
{  
     return normalize(texture(normalMap, coord).xyz*2.0  - 1.0);  
}

vec3 posFromDepth(vec2 coord){
     float d = texture(normalMap, coord).w;
     vec4 tray = mat4(inverseProjectionMatrix) *vec4((coord.x-0.5)*2.0,(coord.y-0.5)*2.0,1.0,1.0);
     return -tray.xyz*d;
}
    //Ambient Occlusion form factor:
float aoFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2){
          vec3 vv = normalize(ddiff);
          float rd = length(ddiff);
          return (1.0-clamp(dot(readNormal(texCoords+vec2(c1,c2)),-vv),0.0,1.0)) *
           clamp(dot( cnorm,vv ),0.0,1.0)* 
                 (1.0 - 1.0/sqrt(1.0/(rd*rd) + 1.0));
    }
    //GI form factor:
 float giFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2){
          vec3 vv = normalize(ddiff);
          float rd = length(ddiff);
          return 1.0*clamp(dot(readNormal(texCoords+vec2(c1,c2)),-vv),0.0,1.0)*
                     clamp(dot( cnorm,vv ),0.0,1.0)/
                     (rd*rd+1.0);  
    }






void main()
{
    //read current normal,position and color.
    vec3 n = readNormal(texCoords.st);
    vec3 p = posFromDepth(texCoords.st);
    vec3 col = texture(colorMap, texCoords).rgb;

    //randomization texture
    vec2 fres = vec2(texSize/128.0*5,texSize/128.0*2)*5.0 - 1.0;
    vec3 random = texture(randomMap, texCoords.st*fres.xy).xyz;
    random = random*2.0-vec3(1.0);

    //initialize variables:
    float ao = 0.0;
    vec3 gi = vec3(0.0,0.0,0.0);
    float incx = 1.0/texSize*0.1;
    float incy = 1.0/texSize*0.1;
    float pw = incx;
    float ph = incy;
    float cdepth = texture(normalMap, texCoords).a;

    //3 rounds of 8 samples each. 
    for(float i=0.0; i<3.0; ++i) 
    {
       float npw = (pw+0.0007*random.x)/cdepth;
       float nph = (ph+0.0007*random.y)/cdepth;

       vec3 ddiff = posFromDepth(texCoords.st+vec2(npw,nph))-p;
       vec3 ddiff2 = posFromDepth(texCoords.st+vec2(npw,-nph))-p;
       vec3 ddiff3 = posFromDepth(texCoords.st+vec2(-npw,nph))-p;
       vec3 ddiff4 = posFromDepth(texCoords.st+vec2(-npw,-nph))-p;
       vec3 ddiff5 = posFromDepth(texCoords.st+vec2(0,nph))-p;
       vec3 ddiff6 = posFromDepth(texCoords.st+vec2(0,-nph))-p;
       vec3 ddiff7 = posFromDepth(texCoords.st+vec2(npw,0))-p;
       vec3 ddiff8 = posFromDepth(texCoords.st+vec2(-npw,0))-p;

       ao+=  aoFF(ddiff,n,npw,nph);
       ao+=  aoFF(ddiff2,n,npw,-nph);
       ao+=  aoFF(ddiff3,n,-npw,nph);
       ao+=  aoFF(ddiff4,n,-npw,-nph);
       ao+=  aoFF(ddiff5,n,0,nph);
       ao+=  aoFF(ddiff6,n,0,-nph);
       ao+=  aoFF(ddiff7,n,npw,0);
       ao+=  aoFF(ddiff8,n,-npw,0);

       gi+=  giFF(ddiff,n,npw,nph)*texture(colorMap, texCoords+vec2(npw,nph)).rgb;
       gi+=  giFF(ddiff2,n,npw,-nph)*texture(colorMap, texCoords+vec2(npw,-nph)).rgb;
       gi+=  giFF(ddiff3,n,-npw,nph)*texture(colorMap, texCoords+vec2(-npw,nph)).rgb;
       gi+=  giFF(ddiff4,n,-npw,-nph)*texture(colorMap, texCoords+vec2(-npw,-nph)).rgb;
       gi+=  giFF(ddiff5,n,0,nph)*texture(colorMap, texCoords+vec2(0,nph)).rgb;
       gi+=  giFF(ddiff6,n,0,-nph)*texture(colorMap, texCoords+vec2(0,-nph)).rgb;
       gi+=  giFF(ddiff7,n,npw,0)*texture(colorMap, texCoords+vec2(npw,0)).rgb;
       gi+=  giFF(ddiff8,n,-npw,0)*texture(colorMap, texCoords+vec2(-npw,0)).rgb;

       //increase sampling area:
       pw += incx;  
       ph += incy;    
    } 
    ao/=24.0;
    gi/=24.0;


    Color = vec4(col-vec3(ao)+gi*5.0,1.0);
	//Color.xyz = vec3(texture(normalMap, texCoords).w);
}