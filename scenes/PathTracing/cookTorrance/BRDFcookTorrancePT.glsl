
#include "scenes/EnvirTexScene/brdf inl/BRDFcookTorrance.glsl"

// path tracing variant
float geometrySchlickGGXPT(vec3 N, vec3 omega, float roughness) {
	float alpha = roughness * roughness;
	float k = alpha / 2.0; // using raster variant would violate energy conservation in a ray tracer
	float NW = max(dot(N, omega), 0.0);
	return NW / (NW * (1.0 - k) + k);
}

// same as raster version just forwarding pt
float geometrySmithPT(vec3 N, vec3 V, vec3 L, float roughness)
{
	float view = geometrySchlickGGXPT(N, V, roughness); // verdeckung von sicht aus (masking)
	float light = geometrySchlickGGXPT(N, L, roughness); // verdeckung von schatten (shadowing)

	return view * light;
}

//TODO https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
vec3 evaluateCookTorranceAndPdf(vec3 V, vec3 L, vec3 N, Material m, float prSpecular, out float outPdf) {
	float NL = max(dot(N, L), 0.0);
	float NV = max(dot(N, V), 0.0);
	if (NL <= 0.0 || NV <= 0.0) {
		outPdf = 0.0;
		return vec3(0.0);
	}

	const float pi = 3.14159265359;
	vec3 H = normalize(V + L);

	vec3 F0 = mix(vec3(0.04), m.c, m.metallic);
	vec3 F = fresnelSchlick(V, H, F0);
	float D = distributionGGX(N, H, m.roughness);

	// Evaluate BRDF Color Components
	float G = geometrySmithPT(N, V, L, m.roughness);
	vec3 diffuseLight = (vec3(1.0) - F) * (1.0 - m.metallic);
	float specDiv = 4.0 * NL * NV;
	vec3 specularLight = (F * D * G) / max(specDiv, 0.001);
	vec3 brdfColor = (diffuseLight * (m.c / pi)) + specularLight;

	// Evaluate PDF Components
	float pdfDiffuse = NL / pi;
	float NH = max(dot(N, H), 0.0);
	float VH = max(dot(V, H), 0.0);
	float pdfSpecular = (D * NH) / (4.0 * max(VH, 0.001));
	
	float prDiffuse = 1.0 - prSpecular;
	outPdf = (prDiffuse * pdfDiffuse) + (prSpecular * pdfSpecular);

	return brdfColor;
}

//TODO could reuse the frame from normal mapping here?
// A small helper to transform a local hemisphere ray to World Space
vec3 toWorld(vec3 N, vec3 localRay) {
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	return localRay.x * tangent + localRay.y * bitangent + localRay.z * N;
}
