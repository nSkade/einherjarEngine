#ifndef _LIGHTDEF_
#define _LIGHTDEF_
/**
 * @param d light distance
 * @param r light area (radius)
*/
float getAttenuation(float d, float r) {
	return max(1.-d*d/(r*r),0.);
}

struct LightType_t {
	int POINT;
	int SUN;
	int SPOT;
};
#ifdef __cplusplus
const LightType_t LightType = { 0, 1, 2 };
#else
const LightType_t LightType = LightType_t(0, 1, 2);
#endif

struct Light {
	vec3 p; // pos
	vec3 c; // col (includes intensity)
	float falloffDist;

	int type;
	vec3 d; // dir for spot and sun
	float cutoff; // spot open angle
};

struct Material {
	vec3 c; // albedo
	vec3 n; // normal
	float roughness;
	float metallic;
};
#endif