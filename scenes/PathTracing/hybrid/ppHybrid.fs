#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

layout(binding=0) uniform sampler2D u_p;
layout(binding=1) uniform sampler2D u_pRaster;

vec3 tonemapNarkowiczACES(vec3 x) {
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	color = vec4(vec3(0.),1.);
	color = texture(u_p,uv);
	//color.xyz /= float(1+u_frame);
	//color.xyz = pow(color.xyz,vec3(0.4545));
	//color.xyz = pow(color.xyz,vec3(1./2.2)); // same as above

	//color.xyz = texture(u_pRaster,uv).xyz;
	// mix
	//color.xyz = texture(u_pRaster,uv).xyz*color.xyz;
#if 1 // acc
	if (u_frame > 0.)
		color.xyz = texture(u_p,uv).xyz/u_frame;
	else
		color.xyz = texture(u_pRaster,uv).xyz;
#endif
#if 0
	color.xyz = texture(u_p,uv).xyz;
#endif

#if 0 // reproj
	color = texture(u_p,uv).rgb;
	//color.rgb /= color.w;
#endif
	//color.xyz = texture(u_pRaster,uv).xyz*color.xyz;

	// will always loose contrast in order to get balanced image
	if (true) { // tonemap
		// Exposure adjustment
		//color.rgb *= 0.1; // exposure with gamma correction leads to washed out image
		//color.rgb *= 0.5; // higher exposer is fine
		color.rgb *= 0.1;
		
		if(false) { // saturation, do rather not use
			float saturation = 1.4;
			const vec3 LumaWeights = vec3(0.2126, 0.7152, 0.0722);
			float luma = dot(color.rgb, LumaWeights);
			color.rgb = vec3(luma) + saturation * (color.rgb - luma);
		}

		// just using aces and exposure is best contrast balance to me
		// aces
		color.rgb = tonemapNarkowiczACES(color.rgb);

		// gamma correction
		color.rgb = pow(color.rgb,vec3(1.0/2.2));
	}
};
