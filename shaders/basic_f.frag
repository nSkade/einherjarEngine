#version 460

uniform float u_time;
uniform vec2 u_resolution;

in vec3 colorV;
in vec3 normalV;

out vec4 color;

void main()
{
	vec3 lightDir = normalize(vec3(1.0,-1.0,1.0));
	//gl_FragColor = vec4(normalTES*0.5+vec3(0.5),1.0);
	color = vec4(normalV,1.0);
	//color = vec4(vec3(gl_FragCoord.z),1.0);
};
