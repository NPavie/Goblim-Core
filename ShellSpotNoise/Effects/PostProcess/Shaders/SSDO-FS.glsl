/*
*	Authors: J. MINIER
*	Master 1 ISICG
*	Source: http://kayru.org/articles/dssdo/
*/

#version 440 core

uniform sampler2DArray GBuffers;
uniform sampler2D depthSampler;
uniform sampler2D noiseSampler;
uniform vec3 kernel[128];

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
    float VStep;
	int numSamples;

	float baseRadius;
	float maxOcclusionDist;
};

layout (location = 0) out vec3 DO;

vec3 ComputeDOFactor(in vec3 origin, in vec3 normal, in vec3 noiseValue )
{
	// calcul du radius pour le sampling, en fonction du depth buffer
	// (plus le fragment est loin, moins le radius est grand)
	float radius = baseRadius / textureLod(depthSampler, texCoord, 0).r;
	// inverse de la distance maximum à partir duquel un sample n'occlude pas un
	// autre
	float maxDistanceInv = 1.0f / maxOcclusionDist;
	float attenuationAngleThreshold = 0.1;

	// calcul des poids pour les harmoniques sphérique
	vec4 occlusionSh2 = vec4(0.0f);
	const float fudgeFactorL0 = 2.0;
	const float fudgeFactorL1 = 10.0;
	const float sh2WeightL0 = fudgeFactorL0 * 0.28209; //0.5*sqrt(1.0/pi);
	const vec3 sh2WeightL1 = vec3(fudgeFactorL1) * 0.48860; //0.5*sqrt(3.0/pi);
	const vec4 sh2Weight = vec4(sh2WeightL1, sh2WeightL0) / numSamples;

	// sampling
	for(int i = 0; i < numSamples; ++i)
	{
		// calcul dans l'espace écran d'un nouveau sample à partir du noise et du radius
		// puis on récupère sa position dans le monde
		vec2 sampleTex = reflect(kernel[i].xy, noiseValue.xy) * radius + texCoord;
		vec4 samplePos4 = textureLod(GBuffers, vec3(sampleTex, 3), 0);
		vec3 samplePos = samplePos4.xyz / samplePos4.w;
	
		// calcul de la distance et du vecteur entre le fragment et le sample
		vec3 originToSample = samplePos - origin;
		float distToSample = length(originToSample);
		vec3 originToSampleNormalized = originToSample / distToSample;

		// calcul de l'attenuation en fonction de la distance du sample, de l'angle
		// entre la normale du fragment et le vecteur fragment->sample
		float attenuation = 1 - clamp(distToSample * maxDistanceInv, 0.0f, 1.0f );
		float dp = dot(normal, originToSampleNormalized);
		attenuation = attenuation * attenuation * step( attenuationAngleThreshold, dp );

		// conversion en harmonique sphérique ?
		vec4 val = attenuation * sh2Weight * vec4(originToSampleNormalized,1);

		// ajout à l'accumulateur si la valeur est valide
		//
		// on test la valeur car sinon celà
		// peut résulter à des beaux glitchs dut à des valeurs infinies
		// qui foutent tout en l'air, car amplifiés par la suite par le blur
		if(!isnan(val.x) && !isinf(val.x))
			occlusionSh2 += val;
	}

	return occlusionSh2.xyz;
}

void main()
{	
	//----- Fragment Position -----//
	vec3 worldPos = textureLod(GBuffers, vec3(texCoord, 3), 0).xyz;
	//----- Fragment Normal -----//
	vec3 worldNormal = textureLod(GBuffers, vec3(texCoord, 4), 0).xyz;

	vec3 noiseValue = textureLod(noiseSampler, texCoord * vec2(HStep, VStep), 0).rgb;
	if(worldNormal != vec3(0.0))
		DO = ComputeDOFactor(worldPos, worldNormal, noiseValue);
	else
		DO = worldPos;
}

