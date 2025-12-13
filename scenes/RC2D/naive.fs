#version 460
// reference https://jason.today/gi

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform int u_mbd;
uniform vec2 u_mouse;

uniform float u_pencilSize;
uniform vec4 u_pencilColor;

uniform int u_rayCount;
uniform int u_raySteps;
uniform float u_rayNoise;
uniform float u_rayDist;
uniform float u_lightStr;

uniform sampler2D u_tex;
uniform sampler2D u_texJumpFlood;

uniform int u_viewPass;

bool outOfBounds(vec2 uv) {
	return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

float rand(vec2 co){
	return fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 raymarch(vec2 uv) {
	vec4 light = texture(u_tex, uv);
	if (light.a > 0.1) {
		return light;
	}
	
	float oneOverRayCount = 1.0 / float(u_rayCount);
	float tauOverRayCount = 3.141592 * 2. * oneOverRayCount;

	// Distinct random value for every pixel, note proper RC doesnt use noise
	float noise = rand(uv)*u_rayNoise;
	
	vec4 radiance = vec4(0.0);

#define JFA 1
#if !JFA
	for(int i = 0; i < u_rayCount; i++) {
		float angle = tauOverRayCount * (float(i) + noise);
		vec2 rayDirectionUv = vec2(cos(angle), -sin(angle))
			/ u_raySteps * u_rayDist;
		vec2 sampleUv = uv;
		for (int j = 0; j< u_raySteps; j++) {
			sampleUv += rayDirectionUv;
			if (outOfBounds(sampleUv))
				break;
			
			vec4 sampleLight = texture(u_tex, sampleUv);
			if (sampleLight.a > 0.5) {
				radiance += sampleLight;
				break;
			}
		}
	}
#endif

#if JFA
	for(int i = 0; i < u_rayCount; i++) {
		float angle = tauOverRayCount * (float(i) + noise);
		vec2 rayDirectionUv = vec2(cos(angle), -sin(angle));
		float ratio = u_resolution.x/u_resolution.y;
		rayDirectionUv.x /= ratio;
		rayDirectionUv = normalize(rayDirectionUv);

		vec2 sampleUv = uv;
		
		for (int j = 0; j< u_raySteps; j++) {
			vec2 nearestSeed = texture(u_texJumpFlood, sampleUv).xy;
			float dist = distance(sampleUv, nearestSeed);
			sampleUv += rayDirectionUv * dist;
			if (outOfBounds(sampleUv))
				break;
			
			if (dist < 0.001) {
				vec4 sampleLight = texture(u_tex, sampleUv);
				radiance += sampleLight * u_lightStr;
				break;
			}
		}
	}
#endif
	return radiance * oneOverRayCount;
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
#if 0
	color = texture(u_tex,uv);
	return;
#endif
	if (u_viewPass==1) {
		color = texture(u_texJumpFlood,uv);
		return;
	}
#if 0
	vec2 nearestSeed = texture(u_texJumpFlood, uv).xy;
	// Clamp by the size of our texture (1.0 in uv space).
	float dist = clamp(distance(uv, nearestSeed), 0.0, 1.0);
	color = vec4(vec3(dist),1.);
	return;
#endif
	
	color = raymarch(uv);
};
