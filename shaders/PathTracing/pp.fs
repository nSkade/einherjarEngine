#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform sampler2D u_p;

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	color = texture(u_p,uv);
	//color.xyz /= float(1+u_frame);
	color.xyz = pow(color.xyz,vec3(0.4545));
};
