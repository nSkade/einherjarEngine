#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 normalTES;

void main()
{
	gl_FragColor = vec4(normalTES*0.5+vec3(0.5),1.0);
};
