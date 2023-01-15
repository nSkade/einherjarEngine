#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 colorTES;
in vec3 normalTES;

out vec3 color;

void main()
{
	//gl_FragColor = vec4(normalTES*0.5+vec3(0.5),1.0);
	color = vec4(colorTES,1.0);
};
