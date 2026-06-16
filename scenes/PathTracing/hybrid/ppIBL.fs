#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

layout(binding=0) uniform sampler2D u_p;
layout(binding=1) uniform sampler2D u_pRaster;
layout(binding=2) uniform sampler2D u_sky;

uniform mat4 u_camInvProj;
uniform mat4 u_camInvView;

vec2 sampleEquirectangular(vec3 v) {
	const vec2 invAtan = vec2(0.1591, 0.3183);
	return (vec2(atan(v.z, v.x), asin(v.y)) * invAtan) + 0.5;
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;

	vec4 v_dir;
	{
		vec4 ndc = vec4(uv*2.-1.,0.,1.);
		vec4 v = u_camInvProj * ndc;
		v = v/v.w;
		v = u_camInvView * v;
		v_dir = v;
		v_dir.xyz -= u_camInvView[3].xyz;
		v_dir.xyz = -normalize(v_dir.xyz);
	}

	color = vec4(vec3(0.),1.);
	//color = texture(u_p,uv);
	//color.xyz /= float(1+u_frame);
	//color.xyz = pow(color.xyz,vec3(0.4545));
	//color.xyz = pow(color.xyz,vec3(1./2.2)); // same as above

	//color.xyz = texture(u_pRaster,uv).xyz;
	// mix
	//color.xyz = texture(u_pRaster,uv).xyz*color.xyz;
	vec4 rt = texture(u_p,uv);
	color.xyz = rt.xyz;
	vec4 raster = texture(u_pRaster,uv);
	color.xyz *= raster.xyz;

	vec2 skyUv = sampleEquirectangular(v_dir.xyz);
	//color.xyz = vec3(skyUv,0.);
	if (rt.a == 0.)
		color.xyz = texture(u_sky,skyUv).xyz;
};
