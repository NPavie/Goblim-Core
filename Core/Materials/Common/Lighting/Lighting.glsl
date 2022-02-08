

struct Light
{
	vec4 pos;
	vec4 color;
	vec4 direction;
	vec4 info;
};

#if __VERSION__ > 410

layout (std430,binding=2) readonly buffer LightingBuffer
{
	vec4 camPos;
	vec4 nbLights;
	Light Lights[];
};

#else 

layout(std140) uniform LightingBuffer
{
	vec4 camPos;
	ivec4 nbLights;	
	Light Lights[100];
};

#endif

// Phong Highlight
vec3 PhongHighlight(in float dotRV, in vec3 S, in vec4 K)
{
	return (S * K.z * pow(dotRV, K.w));
}

// Blinn-Phong Highlight
vec3 BlinnPhongHighlight(in float dotNH, in vec3 S, in vec4 K)
{
	return (S * K.z * pow(dotNH, K.w));
}

// Ward's Anisotropic Highlight
vec3 WardHighlight(in float dotNL, in vec3 V, in vec3 H, in vec3 T, in vec3 B, in vec3 N, in vec3 S, in vec4 K)
{
	vec2 roughness = vec2(0.5, 0.5);
	float PI = 3.1415927410125732421875;
	
	float dotNV = max(dot(N, V), 0.0);

	if(dotNL < 0.0 || dotNV < 0.0)
		return vec3(0.0);
	else
	{
		float dotNH = max(dot(N, H), 0.0);
		float dotHT = max(dot(H, T), 0.0);
		float dotHB = max(dot(H, B), 0.0);
		/*(1.0 / sqrt(dotNL * dotNV)) * (dotNL / (4.0 * PI * roughness.x * roughness.y)) */
		float Ward =  exp(-2.0 * ((pow((dotHT / roughness.x), 2.0) + pow((dotHB / roughness.y), 2.0)) / (1.0 + dotNH)));
		
		return (S * K.z * sqrt(max(0.0, dotNL / dotNV)) * Ward);
	}
}

vec3 Shading(in vec3 T, in vec3 B, in vec3 N, in vec3 V, in vec3 P, in float AO, in vec3 D, in vec3 S, in vec4 K, in int SpecularHighlight)
{
	float Falloff = 1.0, dotNL;
	vec3 L, Ambient = vec3(0.0), Diffuse = vec3(0.0), Specular = vec3(0.0);
	
	Ambient = D * K.x;
	for (int i = 0; i < nbLights.x; i++)
	{
		L = normalize(Lights[i].pos.xyz - P);

		if(Falloff > 0.0)
		{
			dotNL = max(dot(L, N), 0.0);

			Diffuse += D * K.y * dotNL * Lights[i].color.rgb;
			
			if(dotNL > 0.0)
			{
				switch(SpecularHighlight)
				{
					case 0: // Phong Highlight
							Specular += PhongHighlight(max(dot(reflect(-L, N), V), 0.0), S, K);
							break;
					case 1: // Blinn-Phong Highlight
							Specular += BlinnPhongHighlight(max(dot(N, normalize(V + L)), 0.0), S, K);
							break;
					case 2: // Ward's Anisotropic Highlight
							Specular += WardHighlight(dotNL, V, normalize(V + L), T, B, N, S, K);
							break;
				}
				Specular *= Lights[i].color.rgb;
			}
		}
	}
	Diffuse = Diffuse / nbLights.x;
	Specular = Specular / nbLights.x;
	return (Ambient + Diffuse + Specular) * AO;
}


float computeSpecular(in vec4 lightDir,	in vec3 eyeDir ,in vec3 normal,float power)
{
	vec3 R = reflect(-lightDir.xyz,normal);
	return pow(max(0.0,dot(R,eyeDir.xyz)),power);
}
float computeDiffuse(in vec4 lightDir,	in vec3 normal)
{
	return max(0.0,dot(lightDir.xyz,normal));	
}

vec4 
addLight(
	in vec4 LightPos, 
	in vec4 LightColor,
	in vec3 position, 
	in vec3 normal,
	in vec4 ambiant, 
	in vec4 diffuse,
	in vec4 specular,
	in vec4 coefs
	)
{
	vec4 _color;
	vec4 lDir;
	lDir.xyz = LightPos.xyz - position;
	lDir.w = length(lDir);
	lDir.xyz = normalize(lDir.xyz);

	vec4 cPos;
	cPos.xyz = normalize(camPos.xyz-position);
	
	_color += coefs.y * computeDiffuse(lDir,normal) * diffuse * LightColor; // * L.color;
	_color += coefs.z * computeSpecular(lDir,cPos.xyz,normal,coefs.w) * specular * LightColor;// * L.color;

	return _color;
}

vec4 addPhong(
              in vec3 position,
              in vec3 normal,
              in vec4 ambiant,
              in vec4 diffuse,
              in vec4 specular,
              in vec4 coefs)
{	

	vec4 _color;

	_color.xyz = ambiant.xyz*coefs.x;

	for (int i = 0;i < nbLights.x ; i++)
	{
		vec4 col = addLight(Lights[i].pos, Lights[i].color, 
						position, normal,
						ambiant,diffuse,specular,coefs);
		_color.xyz += Lights[i].color.w*col.xyz;			
	}

	return _color;
		
}

/**
 @brief phong illumination function with limited subfunction access (works on iris pro)
 @param position position of the evaluated surface point (in world space)
 @param normal normal of the point (still in world space)
 @param ambiant color of the material when non-directly illuminated (typically the object color)
 @param diffuse color of the material when directly illuminated (typically the object color)
 @param specular color of the material when strongly exposed to light (typically the Light color or white)
 @param coefs coefficents of each color contribution + power of the specular dot
 */
vec4 phongFunction(
    vec3 position,
    vec3 normal,
    vec4 ambiant,
    vec4 diffuse,
    vec4 specular,
    vec4 coefs)
{

    vec4 surfaceColor = vec4(1.0);
    surfaceColor.xyz = ambiant.xyz*coefs.x;

    for(int i = 0; i < nbLights.x; ++i)
    {
        vec4 lightColorReceived = vec4(1.0);
        vec4 lDir;
        lDir.xyz = Lights[i].pos.xyz - position;
        lDir.w = length(lDir);
        lDir.xyz = normalize(lDir.xyz);

        vec4 cPos = vec4(1.0);
        cPos.xyz = normalize(camPos.xyz-position);

        vec3 reflectedLight = reflect(-lDir.xyz,normal);
        float specPower = pow(max(0.0,dot(reflectedLight,cPos.xyz)),coefs.w);
        float diffusePower = max(0.0,dot(lDir.xyz,normal));

        lightColorReceived = (coefs.y * diffusePower * diffuse * Lights[i].color );
        lightColorReceived += (coefs.z * specPower * specular * Lights[i].color );
        
        surfaceColor.xyz += lightColorReceived.xyz * Lights[i].color.w;
    }
    
    return surfaceColor;
    
}

vec4 addBoulanger	(	vec3 position, 
						vec4 positionColor,
						vec3 normal,	
						float occlusionFactor, 
						float reflectance, 
						float attenuation, 
						float emittance  
					) 
{
	vec4 color_added = positionColor;
	color_added.xyz *= reflectance * occlusionFactor;
	
	for (int i = 0;i < nbLights.x;i++){
	
		vec3 L = Lights[i].pos.xyz - position;
		float d = length(L);
		L = normalize(L);
		vec3 N = normalize(normal);
		vec3 Id = Lights[i].color.xyz;
	
		vec3 diffuse_in = reflectance * 
							max(dot(N,L),0) * 
							(Id / (1.0 + attenuation * d * d));
		vec3 diffuse_out = reflectance * emittance * 
							max(dot(-N,L),0) * 
							(Id / (1.0 + attenuation * d * d));

		color_added.xyz += clamp(diffuse_in + diffuse_out , vec3(0),vec3(1));


	}
	
	
	return color_added;

}

vec4 addBoulanger2    ( vec3 position,
                        vec4 positionColor,
                        vec3 normal,
                        vec3 groundNormal,
                        float height,
                        float reflectance,
                        float attenuation,
                        float emittance,
                        float occlusionDensity
                    )
{


    vec4 color_added = positionColor;
    color_added.rgb *= 0.2 * reflectance;
	//

	for (int i = 0;i < nbLights.x;i++)
    {

    	vec3 L = Lights[i].pos.xyz - position;
    	//vec3 L = vec3(0,-10,10) - position;
        float d = length(L);
        L = normalize(L);
        vec3 N = normalize(normal);
        vec3 No = normalize(groundNormal);
        vec3 Id = Lights[i].color.xyz;

        vec3 LL = L*L;

        float volOcclusion = clamp(exp(-occlusionDensity*(-height*dot(No,L) + sqrt(1.0-pow(height,2.0)*(1.0 - dot(No,LL))))),0.01,1.0);

        
        vec3 diffuse_in = reflectance 
        					* max(dot(N,L),0) 
        					* Id * color_added.xyz
        					//* (Id / (1.0 + attenuation * d * d))
        					//* (Id / (1.0 + attenuation))
        					;
        vec3 diffuse_out = reflectance 
        					* max(dot(-N,L),0) 
        					* Id * color_added.xyz
        					//* (Id / (1.0 + attenuation * d * d))
        					//* (Id / (1.0 + attenuation))
        					;

    	//color_added.rgb *= 0.3 * reflectance * volOcclusion;    
        color_added.rgb += (diffuse_in + diffuse_out) * volOcclusion;
        color_added.rgb *= (attenuation * pow(volOcclusion,emittance) );

    
     
    }


    return color_added;
}

vec4 addBoulanger3 ( vec3 position,
					vec4 positionColor,
					vec3 normal,
					vec3 groundNormal,
					float height,
					float reflectance,
					float attenuation,
					float emittance,
					float occlusionDensity
					)
{
	vec4 color_added  = vec4(0.0);
	// test
	color_added.rgb = emittance * positionColor.rgb;
	

	bool barbare = false;
	vec3 N = normalize(normal);

	for (int i = 0;i < nbLights.x;i++)
	{

		vec3 L = Lights[i].pos.xyz - position;
		//L = vec3(0.0,0.0,1.0);
		float d = length(L);
		L = normalize(L);
		
		vec3 Id = Lights[i].color.xyz * Lights[i].color.w;

		vec3 LL = L * L;

		float volOcclusion = clamp( max(exp(-occlusionDensity * (-height * dot(groundNormal,L) + sqrt(1.0-pow(height,2.0)*(1.0 - dot(groundNormal,LL))))),0.0),0.0,1.0);
		volOcclusion = pow(volOcclusion,2.0);
		//volOcclusion *= clamp(abs(dot(N,groundNormal)),0.1, 1.0);
		//volOcclusion *= clamp( abs(  max(dot(N,groundNormal),0.0) + max(dot(-N,groundNormal),0.0) ) ,0.1, 1.0);

		if (barbare)
		{
			color_added.rgb += volOcclusion * positionColor.rgb;
			barbare = true;
		}
		
		//color_added.xyz *= 0.3*reflectance * volOcclusion;

		vec3 diffuse_in = 	reflectance 
							* max(dot(N,L),0) 
							* Id * positionColor.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;
		vec3 diffuse_out = 	reflectance 
							* max(dot(-N,L),0) 
							* Id * positionColor.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;

		color_added.xyz += volOcclusion * attenuation * (diffuse_in + diffuse_out);


		//color_added.xyz = vec3(volOcclusion,0.0,0.0);
	}

	color_added.w = positionColor.w;
	
	return color_added;
}

vec4 addBoulangerFixe ( vec3 position,
					vec4 positionColor,
					vec3 normal,
					vec3 groundNormal,
					float height,
					float reflectance,
					float attenuation,
					float emittance,
					float occlusionDensity
					)
{
	vec4 color_added = positionColor;


	
		vec3 L = vec3(150,150,150) - position;
		//L = vec3(0.0,0.0,1.0);
		float d = length(L);
		L = normalize(L);
		vec3 N = normalize(normal);
		vec3 Id = vec3(1.0);

		vec3 LL = L*L;

		float volOcclusion = clamp( max(exp(-occlusionDensity * (-height * dot(groundNormal,L) + sqrt(1.0-pow(height,2.0)*(1.0 - dot(groundNormal,LL))))),0.0),0.0,1.0);
		volOcclusion = pow(volOcclusion,2.0);
		color_added.xyz *= 0.3 * reflectance * volOcclusion;

		vec3 diffuse_in = reflectance 
							* max(dot(N,L),0) 
							* Id * color_added.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;
		vec3 diffuse_out = reflectance
							* emittance 
							* max(dot(-N,L),0) 
							* Id * color_added.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;

		color_added.xyz += volOcclusion * (diffuse_in + diffuse_out);


		L = vec3(-150,-150,150) - position;
		//L = vec3(0.0,0.0,1.0);
		 d = length(L);
		L = normalize(L);
		 N = normalize(normal);
		 Id = vec3(1.0);

		 LL = L*L;

		 volOcclusion = clamp( max(exp(-occlusionDensity*(-height*dot(groundNormal,L) + sqrt(1.0-pow(height,2.0)*(1.0 - dot(groundNormal,LL))))),0.0),0.0,1.0);
		volOcclusion = pow(volOcclusion,2.0);
		color_added.xyz *= 0.3*reflectance * volOcclusion;

		diffuse_in = reflectance 
							* max(dot(N,L),0) 
							* Id * color_added.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;
		diffuse_out = reflectance
							* emittance 
							* max(dot(-N,L),0) 
							* Id *color_added.xyz ;
							//* (Id / (1.0 + attenuation * d * d))
							//* (Id / (1.0 + attenuation))
							;

		color_added.xyz += volOcclusion*(diffuse_in + diffuse_out);


		//color_added.xyz = vec3(volOcclusion,0.0,0.0);
	


	return color_added;
}