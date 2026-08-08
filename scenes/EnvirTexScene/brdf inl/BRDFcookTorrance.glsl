
#include "LightDef.glsl"

vec3 fresnelSchlick(vec3 V, vec3 H, vec3 F0) {
	//float cosTheta = max(dot(H,V),0.0); //TODO max required?
	float cosTheta = (dot(H,V)); //TODO max required?
	return F0 + (1.0 - F0) * pow(1.0-cosTheta,5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
	float NH = max( dot(N,H) , 0.0);
	float A = (roughness*roughness-1.0);
	const float pi = 3.14159;
	float ret = roughness*roughness / (pi* pow(NH*NH*A+1.0 ,2.0));
	return ret;
}

float geometrySchlickGGX(vec3 N, vec3 omega, float roughness) {
	float k = (roughness*roughness+2.0*roughness+1.0)/8.0; // from UE4, looked good for artists
	float NW = max(dot(N,omega), 0.0);
	float ret = NW / (NW*(1.0-k)+k);
	return ret;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float view = geometrySchlickGGX(N, V, roughness); // verdeckung von sicht aus (masking)
	float light = geometrySchlickGGX(N, L, roughness); // verdeckung von schatten (shadowing)

	return view * light;
}

/**
 * @param pos world space pos of fragment
 * @param camPos 
*/
vec3 BRDFcookTorrance(vec3 pos, vec3 camPos, Light l, Material m) {
	vec3 N=m.n;
	vec3 L=normalize(l.p-pos);
	vec3 V=camPos-pos; //vector world space pos to viewer
	V=normalize(V);
	
	vec3 albedo = m.c;
	
	float lightDistance = length(l.p-pos);

	if (l.type==LightType.SUN) {
		L=l.d;
		lightDistance = 0.;
	}

	vec3 H = normalize(V + L); // halfway

	// Compute the attenuation.
	float attenuation = 1.0;
	if (l.type==LightType.SPOT) {
		const vec3 spotDir = normalize(l.d);
		float theta = max(dot(spotDir, -L), 0.0);

		float phi_op = l.cutoff; //3.14159 * .5; // open angle
		float alpha_exp = 1.; // radiation exponent
		if (theta > phi_op)
			attenuation = pow(theta, alpha_exp);
		else
			attenuation = 0.0;
	}
	
	attenuation *= getAttenuation(lightDistance, l.falloffDist);
	
	// Mix a dark-ish color with the albedo, based on the metallicness of the material (this makes metallic surfaces appear brighter by default)
	vec3 F0 = mix(vec3(0.03), albedo, m.metallic);

	vec3 F = fresnelSchlick(V, H, F0);
	float D = distributionGGX(N, H, m.roughness);
	float G = geometrySmith(N, V, L, m.roughness);
	
	vec3 diffuseLight = (vec3(1.0) - F)*(1.0-m.metallic);
	
	
	// computes brdf
	float specDiv = max(4.*max(dot(N,L),0.)*max(dot(N,V),0.), .001);
	specDiv = max(specDiv,0.001); // avoid 0 division
	vec3 specularLight = F*D*G/specDiv;

	// Compute the contribution of the light.
	const float pi = 3.14159;
	vec3 radiance = l.c * attenuation;
	vec3 ret = ((diffuseLight * (albedo / pi)) + specularLight) * radiance;
	ret = ret * max(dot(N, L), 0.0);
	return ret;
}
