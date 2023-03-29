#version 460
uniform vec2 u_resolution;
uniform float u_time;

in vec3 colorV;
in vec3 normalV;

out vec4 color;

#include "gradientNoise2D.glsl"

vec3 rgb255(float r, float g, float b) {
	return vec3(r,g,b)/255.0;
}

bool even(float x) {
	return mod(floor(x),2.0)==0.0;
}

vec3 bellCol(float x, vec3 c) {
	float i = abs(x-0.7)*2.0;
	i=1.0-i*i;
	return vec3(x)*(1.0-i)+i*c*x;
}

void main () {
	vec2 st = gl_FragCoord.xy/u_resolution.xy;
	vec2 uv = st;
	float aspect = u_resolution.x/u_resolution.y;
	st.x *= aspect;
	st.y -= u_time*0.05;
	
	float n = 0.0;
	n = gradientNoise(st*10.0);
	n = n*gradientNoise(st*10.01)+n*0.5;
	
	vec3 c = rgb255(250.0,177.0,37.0);
	
	n = n*7.5+u_time*0.3;
	float g = mod(n,1.0);
	//g = smoothstep(0.0,1.0,g);
	if (even(n))
		g = 1.0-g;
	vec3 color2 = bellCol(g,c);
	float x = (g-0.35);
	float redArea = exp(-20.0*x*x);
	color2 += redArea*vec3(0.2,0.0,0.0);
	
	color = vec4(color2,1.0);
}
