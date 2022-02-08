#version 440 core


uniform sampler2D ColorSampler;
uniform sampler2DArray GBuffers;

  in vec2 texCoord;

  layout (location = 0) out vec3 Color;

  layout(std140) uniform CPU {
  	float velocityScale; // frameRate / targetedFrameRate
  };


  void main()
  {


    if( textureLod(GBuffers, vec3(texCoord, 3.0), 0).w != 1.0 ) // Si position monde != 1.0 (masque pour application de l'effet ou non)
      discard;
    

    vec2 texelSize = 1.0 / vec2(textureSize(ColorSampler, 0)); // Taille d'un texel
  
    vec2 velocity = textureLod(GBuffers, vec3(texCoord, 5.0), 0).rg; // Lecture de la vélocité
    velocity *= velocityScale; // Scale par rapport au frameRate

    float speed = length(velocity / texelSize); // Velocité en texels
    int nbSamples = clamp(int(speed), 1, 16); // Limite le nombre de samples

    Color = textureLod(ColorSampler, texCoord, 0).rgb; // Couleur au pixel considéré

    for (int i = 1; i < nbSamples; i++) { // On sample le long du vecteur de vélocité

      vec2 offset = velocity * (float(i) / float(nbSamples - 1) - 0.5);
      vec2 uv2 = texCoord + offset;
      if(uv2.x>0.0 && uv2.x<1.0 && uv2.y>0.0 && uv2.y<1.0) { // Si on ne sort pas de la texture
        Color += textureLod(ColorSampler, uv2, 0).rgb;
      } /*else { nbSamples = i; break; }*/
    
    }
    Color /= float(nbSamples);


  }

