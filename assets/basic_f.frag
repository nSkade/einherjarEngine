#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 normalTES;

void main()
{
	gl_FragColor = vec4(normalTES,1.0);
};
