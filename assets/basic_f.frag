#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 color;

void main()
{
	gl_FragColor = vec4(color,1.0);
};
