/*
*	Authors: J. MINIER
*	Master 1 ISICG
*	Source: http://kayru.org/articles/dssdo/
*/

#version 440 core

uniform sampler2DArray GBuffers;
uniform sampler2D DepthSampler;
uniform sampler2D NoiseSampler;
uniform vec3 kernel[128];

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
    float VStep;
	int numSamples;

	float baseRadius;
	float maxOcclusionDist;
	/*int version;
	*/

};

layout (location = 0) out vec3 DO;

vec3 ComputeDOFactor(in vec3 Origin, in vec3 Normal, in vec3 NoiseValue )
{
	vec3 Sample;
	float SampleDepth;

	vec3 OccluderPos;
	vec3 OccluderNorm = vec3(0);
	vec3 OccluderNormSum = vec3(0);

	vec3 radianceSample;
	float depthDiff;

	float Radius = baseRadius / textureLod(DepthSampler, texCoord, 0).r;

	vec3 directLightning = vec3( 0.0f );
	vec3 indirectLightning = vec3( 0.0f );
	float receiverGeometricTerm;

	float max_distance_inv = 1.0f / 10;
	float attenuation_angle_threshold = 0.1;

	vec4 occlusion_sh2 = vec4(0.0f);
	const float fudge_factor_l0 = 2.0;
	const float fudge_factor_l1 = 10.0;

	const float sh2_weight_l0 = fudge_factor_l0 * 0.28209; //0.5*sqrt(1.0/pi);
	const vec3 sh2_weight_l1 = vec3(fudge_factor_l1) * 0.48860; //0.5*sqrt(3.0/pi);

	const vec4 sh2_weight = vec4(sh2_weight_l1, sh2_weight_l0) / numSamples;

	for(int i = 0; i < numSamples; ++i)
	{
		vec2 sample_tex = reflect(kernel[i].xy, NoiseValue.xy) * Radius + texCoord;
		vec4 sample_pos4 = textureLod(GBuffers, vec3(sample_tex, 3), 0);
		vec3 sample_pos = sample_pos4.xyz / sample_pos4.w;
	
		vec3 center_to_sample = sample_pos - Origin;
		
		float dist = length(center_to_sample);
		vec3 center_to_sample_normalized = center_to_sample / dist;

		float attenuation = 1 - clamp(dist * max_distance_inv, 0.0f, 1.0f );

		float dp = dot(Normal, center_to_sample_normalized);
		
		attenuation = attenuation * attenuation * step( attenuation_angle_threshold, dp );
		vec4 val = attenuation * sh2_weight * vec4(center_to_sample_normalized,1);

		if(!isnan(val.x) && !isinf(val.x))
			occlusion_sh2 += val;


		//continue;
		//if (version == 1)
		//	return Normal;
		//if (version == 2)
		//	OccluderNorm += vec3( step( attenuation_angle_threshold, dp ) );
		//if (version == 3)
		//OccluderNorm += vec3( attenuation );
		//if (version == 4)
		//	OccluderNorm += vec3( attenuation );
			
	 // ---------------------------------
		//Sample = Radius * reflect(kernel[i], NoiseValue);
		//Sample.xyz = sign(dot(Sample,Normal)) * Sample.xyz;
		//vec3 SampleDir = normalize( sample_pos - Origin );
		//Sample += Origin;
		//SampleDepth = texture(DepthSampler, Sample.xy).r;


		//float distanceTerm = abs(SampleDepth - Sample.z) < 0.1 ? 1.0 : 0.0;
	 // float visibility = 1.0 - 1.0 * (Sample.z > SampleDepth ? 1.0 : 0.0) * distanceTerm;
	 // float receiverGeometricTerm = max(0.0, dot(SampleDir, Normal));
	 // vec3 senderRadiance = vec3 (1);
	 // vec3 radiance = visibility * receiverGeometricTerm * senderRadiance;


	 // ---------------------------------

		//vec3 SamplePos = vec3( Sample.xy, SampleDepth );
		//float RadiusSample = Radius;// / Origin.z;
		//vec2 sample_tex = reflect(kernel[i].xy, NoiseValue.xy) * RadiusSample;
		//vec4 sample_pos4 = textureLod(GBuffers, vec3(sample_tex, 3), 0);
		//vec3 sample_pos = sample_pos4.xyz / sample_pos4.w;
		//vec3 sample_normal = textureLod(GBuffers, vec3(sample_tex, 4), 0).xyz;
		
		//vec3 center_to_sample = sample_pos - Origin; 
		//float dist  = length( center_to_sample );
		//vec3 center_to_sample_normalized = center_to_sample / dist;
		//float dp = dot( Normal, center_to_sample_normalized );
		// distanceTerm: permet d'enlever les cas des bords où le sample et le point d'origine sont éloignés
		// et ne sont pas à considérer comme des occluders (par ex les bords)
		// 0 si le sample n'est pas à prendre en compte
		// 1: oui
		//float distanceTerm = 0.0;
		//if (version == 2)
			//distanceTerm = abs(Origin.z - SampleDepth) < 0.01f ? 1.0f : 0.0f;
		//distanceTerm = dist < 0.3f ? 1.0f : 0.0f;
		//float distanceTerm = abs(Origin.z - SampleDepth) < RadiusSample /*singularity*/ ? 1.0 : 0.0;
		//dist = dist < 1 ? dist : 0;
		//float distanceTerm = clamp( dist * maxDistInv, 0, 1 );
		// visibility: le sample est au dessus de la surface et valide
		// 1 = visible
		// 0 =invisible
		//float visibility = 1;// - /*strength*/1.0f * ( Sample.z > SampleDepth ? 1.0 : 0.0) * distanceTerm;
			
		//directLightning += abs(Origin.z - SampleDepth) < 0.01f ? 1.0f : 0.0f;

		
	}

	return occlusion_sh2.xyz;
}

void main()
{	
	
		//----- Fragment Depth -----//
		float OriginDepth = textureLod(DepthSampler, texCoord, 0).r;
		//----- Fragment Position -----//
		vec3 Origin = vec3(texCoord, OriginDepth);
		vec3 WorldPos = textureLod(GBuffers, vec3(texCoord, 3), 0).xyz;
	
		vec3 NoiseValue = textureLod(NoiseSampler, texCoord * vec2(HStep, VStep), 0).rgb;
		//----- Fragment Normal -----//
		vec3 RelativeNormal = textureLod(GBuffers, vec3(texCoord, 4), 0).xyz;
		vec3 ViewNormal = textureLod(GBuffers, vec3(texCoord, 5), 0).xyz;

		if(RelativeNormal != vec3(0.0) && RelativeNormal != vec3(1.0) && OriginDepth < 1.0)
			DO = ComputeDOFactor(WorldPos, RelativeNormal, NoiseValue);
		else
		{	
			if(RelativeNormal == vec3(1.0))
				DO = vec3(1.0);
			else
				DO = vec3(0.0,0.0,0.0);
		}

}

/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/
/*
#version 440 core

uniform sampler2DArray GBuffers;
uniform sampler2D DepthSampler;
uniform sampler2D NoiseSampler;
uniform vec3 kernel[128];

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
    float VStep;
	int numSamples;
	int version;
};

layout (location = 0) out vec3 AO;

vec3 ComputeAOFactor(in vec3 Origin, in vec3 Normal, in vec3 NoiseValue, in float OriginDepth)
{
	vec3 Sample;
	float SampleDepth;

	float depthDiff;

	float Radius = 0.03125;
	vec3 Occlusion = vec3(0.0);

	for(int i = 0; i < numSamples; ++i)
	{
		Sample = Radius * reflect(kernel[i], NoiseValue);
		Sample = Origin + sign(dot(Sample,Normal)) * Sample;

		SampleDepth = texture(DepthSampler, Sample.xy).r;

		vec3 SamplePos = vec3( Sample.xy, SampleDepth );
		float dist = distance( SamplePos, Origin);

		vec3 dir = normalize( SamplePos - Origin );

		depthDiff = OriginDepth - SampleDepth ;

		if(depthDiff > 0.0 && version == 2)
			Occlusion += vec3(1.0 - smoothstep(0.0, Radius, depthDiff));
		else if (version == 3)
			Occlusion += dir;
	}

	return vec3(1.0 - (Occlusion / float(numSamples)));
}

void main()
{	
	//----- Fragment Depth -----//
	float OriginDepth = texture(DepthSampler, texCoord).r;
	//----- Fragment Position -----//
	vec3 Origin = vec3(texCoord, OriginDepth);
	//----- Fragment Noise Value -----//
	vec3 NoiseValue = texture(NoiseSampler, texCoord * vec2(HStep, VStep)).rgb;
	//----- Fragment Normal -----//
	vec3 Normal = texture(GBuffers, vec3(texCoord, 4.0)).xyz;

	if(Normal != vec3(0.0) && Normal != vec3(1.0) && OriginDepth < 1.0)
			AO = vec3(1.0, 0.0, 1.0);//vec3(ComputeAOFactor(Origin, Normal, NoiseValue, OriginDepth));
	else
	{	
		if(Normal == vec3(1.0))
			AO = vec3(1.0);
		else
			AO = vec3(0.0);
	}
}*/