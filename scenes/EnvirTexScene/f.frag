#version 460

uniform float u_time;
uniform vec2 u_resolution;

layout(binding = 0) uniform sampler2D u_albedo;
layout(binding = 1) uniform sampler2D u_normal;

in vec3 posV;
in vec3 colorV;
in vec3 normalV;
in vec2 uvV;

out vec4 color;

void main()
{
	vec3 lightPos = vec3(0.,3.*sin(u_time)+3.,0.);
	vec3 lightDir = normalize(lightPos-posV);

	//vec3 n =texture(u_normal,uvV).xyz*2.-1.;
	//n=normalize(n);

	vec4 n=vec4(normalV,0.);
	float i =dot(n.xyz,lightDir);//*.5+.5;
	color = vec4(vec3(i),1.0);
	color = vec4(uvV,0.,1.);
	color = texture(u_albedo,uvV) * i;
};
