#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 colorV;
in vec3 normalV;

void main()
{
	//gl_FragColor = vec4(normalTES*0.5+vec3(0.5),1.0);
	gl_FragColor = vec4(colorV,1.0);
}