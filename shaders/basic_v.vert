#version 460

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNrm;
layout(location = 2) in vec3 vCol;

uniform mat4 MVP;
uniform mat4 u_view;

out vec3 colorV;
out vec3 normalV;

void main()
{
	gl_Position = MVP * u_view * vec4(vPos.xz,vPos.y, 1.0);
	colorV = vec3(0.4);//vec3(vCol,1.0);
	normalV = vec3(0.0f,vNrm.z,0.0f);
};
