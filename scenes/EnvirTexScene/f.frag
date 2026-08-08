#version 460

uniform float u_time;
uniform vec2 u_resolution;

layout(binding = 0) uniform sampler2D u_albedo;
layout(binding = 1) uniform sampler2D u_normal;
layout(binding = 2) uniform sampler2D u_mat;

uniform mat4 u_v; // view

in vec3 posV;
in vec3 colorV;
in vec3 normalV;
in vec2 uvV;
in mat3 TBN;
in mat4 pv;

out vec4 color;

#include "brdf inl/LightDef.glsl"
#include "brdf inl/BRDFcookTorrance.glsl"
#include "brdf inl/blinnPhong.glsl"

void main() {
	color=vec4(vec3(0.),1.);
	vec4 albedoAlpha = texture(u_albedo,uvV);
	if (albedoAlpha.a==0.)
		discard;

	Light l; {
		l.p= vec3(-1.,1.,1.);
		l.c=vec3(2.);
			// for pbr, light color contains also radiance strength so can be > 1 for each channel
		l.falloffDist=10.;
		l.d=vec3(1.,0.,0.);
		
		// lightPos
		//l.p= vec3(0.,3.*sin(u_time)+3.,0.);
		l.p= vec3(3.*cos(u_time),4.*sin(u_time)+5.,0.);
	}
	
	Material mat; {
		mat.c = albedoAlpha.xyz;
		mat.n =TBN*(texture(u_normal,uvV).xyz*2.-1.);
		mat.n=normalize(mat.n);

		vec2 m = texture(u_mat,uvV).gb; // roughness, metallic
		mat.roughness=m.x;
		mat.metallic=m.y;
	}

	vec3 camPos=vec3(inverse(u_v)[3].xyz);
	//l.pos=camPos;

	//vec3 n=normalV;
	vec2 uv =gl_FragCoord.xy/u_resolution;
	//if (uv.x < .5)
	//	n=normalV;

	if (false) {
		vec3 lightDir = normalize(l.p-posV);
		vec3 n =normalV;
		float i =dot(n,lightDir);//*.5+.5;
		color = vec4(vec3(i),1.0);
		//color = vec4(uvV,0.,1.);
		//color = texture(u_albedo,uvV) * i;
	}

	if (false) { // blinn phong
		//vec3 N=mat.n;
		//vec3 L=lightDir;
		//vec3 V=camPos-posV; //vector world space pos to viewer
		//V=normalize(V);

		//color = vec4(blinnPhong(posV,camPos,l,mat),1.);
	}

	if (true) { // cook torrance brdf
		color.rgb += BRDFcookTorrance(posV,camPos,l,mat);
	}

	if (false) {
		// l2
		
		Light l2; {
			l2.p= vec3(0.,0.,0.);
			l2.c=vec3(1.);
			l2.type=LightType.SUN;
			l2.falloffDist=100.;
			//l2.d=vec3(1.,1.,0.);
			l2.d=vec3(0.,-1.,0.);
		}
		color.rgb += BRDFcookTorrance(posV,camPos,l2,mat);
	}

	{
		vec4 lightPosSS=pv*vec4(l.p,1.);
		lightPosSS /= lightPosSS.w;

		vec4 camDir = inverse(u_v)*vec4(0.,0.,-1.,0.);
		bool lightInView = dot(normalize(camDir.xyz),normalize(l.p-camPos)) > 0.;

		if (lightInView && length((lightPosSS*.5+.5).xy - uv) < .003)
			color = vec4(1.);
	}
	//color = vec4(n,1.);
};
