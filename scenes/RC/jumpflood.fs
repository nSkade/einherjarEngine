#version 460
// reference https://jason.today/gi

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform sampler2D u_tex;
uniform sampler2D u_texJumpFlood;

uniform float u_jfOffset;

// computes distance field from 
void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	if (u_jfOffset==0.) {
		float alpha = texture(u_tex,uv).a;
		color = vec4(uv * alpha,0.,1.);
		return;
	}

	vec4 nearestSeed = vec4(-2.0);
	float nearestDist = 999999.9;
	
	for (float y = -1.0; y <= 1.0; y += 1.0) {
		for (float x = -1.0; x <= 1.0; x += 1.0) {
			vec2 sampleUV = uv + vec2(x, y) * u_jfOffset / u_resolution;
			if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
				continue;

			vec4 sampleValue = texture(u_texJumpFlood, sampleUV);
			vec2 sampleSeed = sampleValue.xy;

			if (sampleSeed.x != 0.0 || sampleSeed.y != 0.0) {
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
};
