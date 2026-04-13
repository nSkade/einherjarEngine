#version 460

uniform float u_time;
uniform vec2 u_resolution;

layout(binding = 0) uniform sampler2D u_albedo;
layout(binding = 1) uniform sampler2D u_normal;

uniform mat4 u_v; // view

in vec3 posV;
in vec3 colorV;
in vec3 normalV;
in vec2 uvV;
in mat3 TBN;
in mat4 pv;

out vec4 color;

/**
 * @param d light distance
 * @param r light area (radius)
*/
float attenuation(float d, float r) {
	return max(1.-d*d/(r*r),0);
}

void main()
{
	vec3 lightPos = vec3(0.,3.*sin(u_time)+3.,0.);
	lightPos = vec3(3.*cos(u_time),4.*sin(u_time)+5.,0.);
	vec3 camPos=vec3(inverse(u_v)[3].xyz);
	//lightPos=camPos;
	
	vec3 lightDir = normalize(lightPos-posV);

	vec3 n =TBN*(texture(u_normal,uvV).xyz*2.-1.);
	n=normalize(n);

	//vec3 n=normalV;
	vec2 uv =gl_FragCoord.xy/u_resolution;
	//if (uv.x < .5)
	//	n=normalV;
	float i =dot(n,lightDir);//*.5+.5;

	if (false) {
		color = vec4(vec3(i),1.0);
		color = vec4(uvV,0.,1.);
		color = texture(u_albedo,uvV) * i;
	}


	if (true) { // blinn phong
		vec3 N=n;
		vec3 L=lightDir;
		vec3 V=camPos-posV; //vector world space pos to viewer
		V=normalize(V);
	
		vec3 k_a = vec3(0.);
		vec3 k_d = vec3(1.) * texture(u_albedo,uvV).xyz;
		vec3 k_s = vec3(.4);
		vec3 I_a = vec3(.0);
		vec3 I_d = vec3(1.);
		vec3 I_s = vec3(1.);
		float s = 50.; // shinyness
		float u = float(dot(N,L)>=0); // to avoid highlights on backside
		vec3 H=(V+L);
		H=normalize(H);

		vec3 I = k_a * I_a
			+ attenuation(length(lightPos-posV),10.) * (k_d * I_d * max(dot(N,lightDir),0)
				+ k_s * I_s * u * pow(max(dot(N,H),0),s));
		color = vec4(I,1.);
	}
	vec4 lightPosSS=pv*vec4(lightPos,1.);
	lightPosSS /= lightPosSS.w;

	vec4 camDir = inverse(u_v)*vec4(0.,0.,-1.,0.);
	bool lightInView = dot(normalize(camDir.xyz),normalize(lightPos-camPos)) > 0.;

	if (lightInView && length((lightPosSS*.5+.5).xy - uv) < .003)
		color = vec4(1.);
	//color = vec4(n,1.);
};
