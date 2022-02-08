  #version 440 core


  uniform sampler2D ColorSampler;
  uniform sampler2D DepthSampler;

  in vec2 uv;

  layout (location = 0) out vec3 Color;

  layout(std140) uniform CPU {
  	mat4 invViewProj;
  	mat4 invViewProjLast;
  };


  void main()
  {

  	// Get the depth buffer value at this pixel.  
    float zOverW = texture(DepthSampler, uv).r ;  
  	// H is the viewport position at this pixel in the range -1 to 1.  
    vec4 H = vec4( uv.x * 2.0 - 1.0, ( 1.0 - uv.y ) * 2.0 - 1.0, zOverW, 1.0 );  
    // Transform by the view-projection inverse.  
    vec4 D = invViewProj * H;  
    // Divide by w to get the world position.  
    vec4 worldPos = D / vec4(D.w);  

    // Current viewport position  
    vec4 currentPos = H;  
    // Use the world position, and transform by the previous view-  
     // projection matrix.  
    vec4 previousPos = invViewProjLast * worldPos;  
    // Convert to nonhomogeneous points [-1,1] by dividing by w.  
  	previousPos /= vec4(previousPos.w);  
    // Use this frame's position and last frame's to compute the pixel  
    // velocity.  
    vec2 velocity = (currentPos.xy - previousPos.xy)*0.5;  


    // Get the initial color at this pixel.  
    int nSamples = 64;
    vec4 result = texture(ColorSampler, uv);
    for (int i = 1; i < nSamples; ++i) {
        // get offset in range [-0.5, 0.5]:
        vec2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
        vec2 uv2 = uv + offset;
        if(uv2.x>0.0 && uv2.x<1.0 && uv2.y>0.0 && uv2.y<1.0) {
          // sample & add to result:
          result += texture(ColorSampler, uv2);
        }
    }
   
    result /= float(nSamples);
  	Color = vec3(result);
  }