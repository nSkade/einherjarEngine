
#include "LightDef.glsl"

/**
 * @param pos world space pos of fragment
 * @param camPos 
*/
vec3 blinnPhong(vec3 pos, vec3 camPos, Light l, Material m) {
	vec3 N=m.n;
	vec3 L=normalize(l.p-pos);
	vec3 V=camPos-posV;
	V=normalize(V);
	
	vec3 k_a = vec3(0.);
	vec3 k_d = vec3(1.) * m.c * m.roughness;
	vec3 k_s = vec3(1.) * m.metallic;
	vec3 I_a = vec3(.0);
	vec3 I_d = vec3(1.);
	vec3 I_s = vec3(1.);
	float s = 50.; // shinyness


	float u = float(dot(N,L)>=0); // to avoid highlights on backside
	vec3 H=(V+L);
	H=normalize(H);

	float spotCoeff = 1.;
	if (l.type==LightType.SPOT) {
		float theta = dot(l.d,-L);
		float alpha_exp = 1.; // radiation exponent
		float phi_op = 3.14159 * .5; // open angle
		spotCoeff=pow(theta,alpha_exp);
	}

	vec3 I = k_a * I_a
		+ spotCoeff*getAttenuation(length(l.p- pos),l.falloffDist) * (k_d * I_d * max(dot(N,L),0)
			+ k_s * I_s * u * pow(max(dot(N,H),0),s));
	return I;
}