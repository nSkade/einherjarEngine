#version 460

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNrm;
layout(location = 2) in vec3 vCol;
layout(location = 3) in vec2 vUV;
layout(location = 4) in vec4 vTan;

uniform mat4 u_p; // proj 
uniform mat4 u_v; // view
uniform mat4 u_m; // model
              //screen<-camera<-world<-object
uniform mat4 u_imtn;// inverse transpose model mat for normal

out vec3 posV;
out vec3 colorV;
out vec3 normalV;
out vec2 uvV;
out mat3 TBN;
out mat4 pv;

void main()
{
	mat4 pvm = u_p*u_v*u_m;
	pv=u_p*u_v;
	vec4 pos = pvm * vec4(vPos, 1.0f);
	gl_Position = pos;
	posV = (u_m*vec4(vPos,1.)).xyz;
	colorV = vec3(1.0f);//vCol;
	normalV = vNrm;
	uvV = vUV;
	{ //compute tbn for normal mapping
		vec3 T=normalize((u_imtn*vec4(vTan.xyz,1.)).xyz);
		vec3 N=normalize((u_imtn*vec4(vNrm,1.)).xyz);
		T = normalize(T - dot(T, N) * N); // Re-orthogonalize
		vec3 B=cross(N,T)*vTan.w;
		TBN=mat3(T,B,N);
	}
};
