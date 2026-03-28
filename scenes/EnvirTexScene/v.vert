#version 460

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNrm;
layout(location = 2) in vec3 vCol;
layout(location = 3) in vec2 vUV;

uniform mat4 u_pvm; // proj * view * model
uniform mat4 u_m; // proj * view * model
              //screen<-camera<-world<-object
uniform mat4 u_imtn;// inverse transpose model mat for normal

out vec3 posV;
out vec3 colorV;
out vec3 normalV;
out vec2 uvV;

void main()
{
	vec4 pos = u_pvm * vec4(vPos, 1.0f);
	gl_Position = pos;
	posV = (u_m*vec4(vPos,1.)).xyz;
	colorV = vec3(1.0f);//vCol;
	normalV = vNrm;
	uvV = vUV;
	//TODO compute tbn for normal mapping
};
