#version 460

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNrm;
layout(location = 2) in vec3 vCol;

uniform mat4 pvm; // proj * view * model
              //screen<-camera<-world<-object

out vec3 colorV;
out vec3 normalV;

void main()
{
	gl_Position = pvm * vec4(vPos, 1.0f);
	colorV = vec3(0.4f);//vec3(vCol,1.0);
	normalV = vec3(0.0f,0.0f,0.0f);
};
