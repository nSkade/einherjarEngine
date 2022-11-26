#version 460

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vCol;
layout(location = 2) in vec3 vNrm;

uniform mat4 MVP;
uniform mat4 u_view;

out vec3 colorV;
out vec3 normalV;

void main()
{
	//gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	gl_Position = vec4(vPos, 0.0, 1.0);
	colorV = vec3(vCol,1.0);
	normalV = vNrm;
}
