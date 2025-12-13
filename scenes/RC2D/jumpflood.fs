#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform ivec2 u_resolution;
uniform float u_jfOffset;

uniform sampler2D u_tex;
uniform sampler2D u_texJumpFlood;

#define NO_SEED_SENTINEL -100.0

void main() {
	vec2 uv = vec2(gl_FragCoord.xy)/u_resolution.xy;

	if (u_jfOffset == 0.0) {
		float alpha = texture(u_tex, uv).a;

		if (alpha > 0.5)
			color = vec4(uv, 0.0, 1.0);
		else
			color = vec4(NO_SEED_SENTINEL, NO_SEED_SENTINEL, 0.0, 1.0);
		return;
	}

	vec4 nearestSeed = texture(u_texJumpFlood, uv);
	vec2 nearestSeedUV = nearestSeed.xy;
	float nearestDist = 9999999.0;

	if (nearestSeedUV.x > NO_SEED_SENTINEL) {
		vec2 diff = nearestSeedUV - uv;
		nearestDist = dot(diff, diff);
	} else
		nearestSeed = vec4(NO_SEED_SENTINEL, NO_SEED_SENTINEL, 0.0, 1.0);
	
	for (float y = -1.0; y <= 1.0; y += 1.0) {
		for (float x = -1.0; x <= 1.0; x += 1.0) {
			if (x == 0.0 && y == 0.0) continue;
			
			vec2 sampleUV = uv + vec2(x, y) * u_jfOffset / u_resolution;
			
			vec4 sampleValue = texture(u_texJumpFlood, sampleUV);
			vec2 sampleSeed = sampleValue.xy;

			if (sampleSeed.x > NO_SEED_SENTINEL) {
				vec2 diff = sampleSeed - uv;
				float dist = dot(diff, diff);
				
				if (dist < nearestDist) {
					nearestDist = dist;
					nearestSeed = sampleValue;
				}
			}
		}
	}
	
	color = nearestSeed;
}
