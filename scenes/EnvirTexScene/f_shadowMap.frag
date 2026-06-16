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
#include "brdf inl/BRDFcookTorrence.glsl"
#include "brdf inl/blinnPhong.glsl"

layout(binding = 3) uniform sampler2D u_shadowMap;
uniform mat4 u_shadowMat;

float getShadow(vec3 pos) {
	vec4 fragPosLightSpace = u_shadowMat * vec4(pos, 1.0); // can move this to vertex shader
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if(abs(projCoords.z) > 1. || abs(projCoords.x) > 1. || abs(projCoords.y) > 1.)
		return 1.; // how shadow is handled outside frustum
	projCoords = projCoords.xyz*.5+.5;

	float bias = 0.000003;

	float ret=0.;
	if (true) { // no blur
		float closestDepth = texture(u_shadowMap, projCoords.xy).r;
		//if (projCoords.z < 0.) // avoid light behing near frustum
		if (closestDepth > projCoords.z - bias)
			return 1.;
	}

	if (false) { //boxblur
		const vec2 shadowRes = vec2(textureSize(u_shadowMap, 0));
		vec2 resOffset = vec2(1.0)/shadowRes;
		int k_size = 5; // box filter size
		int corr = k_size/2;
	
		// test // depth dependent penumbra approx
		////vec2 uv =gl_FragCoord.xy/u_resolution;
		//if (true) {
		//	float closestDepth = texture(u_shadowMap, projCoords.xy).r;
		//	float depth = (closestDepth - projCoords.z)*700.+2.;
		//	resOffset *= depth;
		//}
		
		for(int x = 0; x < k_size; x++) {
			for(int y = 0; y < k_size; y++) {

				float x_off = (x-corr) * resOffset.x;
				float y_off = (y-corr) * resOffset.y;

				vec3 c = projCoords + vec3(x_off,y_off,0.);
				float closestDepth = texture(u_shadowMap, c.xy).r;
				if (closestDepth > projCoords.z - bias)
					ret += 1.0/(k_size*k_size);
			}
		}
	}

	return ret;
}

void main() {
	color=vec4(vec3(0.),1.);
	vec4 albedoAlpha = texture(u_albedo,uvV);
	if (albedoAlpha.a==0.)
		discard;

	Light l; {
		//l.p= vec3(0.,1.,0.);
		//l.type=LightType.SUN;
		l.c=vec3(5.);
			// for pbr, light color contains also radiance strength so can be > 1 for each channel
		l.falloffDist=20.;
		//l.d=vec3(0.,1.,0.);
		
		// lightPos
		//l.p= vec3(0.,3.*sin(u_time)+3.,0.);
		//l.p= vec3(3.*cos(u_time),4.*sin(u_time)+5.,0.);

		l.cutoff=3.141592*.225;
		l.type=LightType.SPOT;
		mat4 invShadowMat = inverse(u_shadowMat);
		l.p=invShadowMat[3].xyz / invShadowMat[3].w;
		{
			vec3 right = invShadowMat[0].xyz;
			vec3 up    = invShadowMat[1].xyz;
			l.d = -normalize(cross(right, up));
		}
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

	vec2 uv =gl_FragCoord.xy/u_resolution;

	if (true) { // ambient
		color.rgb += albedoAlpha.rgb*.1;
	}

	//if (true) { // cook torrence brdf
	//	color.rgb += BRDFcookTorrence(posV,camPos,l,mat);
	//}

	{
		vec4 lightPosSS=pv*vec4(l.p,1.);
		lightPosSS /= lightPosSS.w;

		vec4 camDir = inverse(u_v)*vec4(0.,0.,-1.,0.);
		bool lightInView = dot(normalize(camDir.xyz),normalize(l.p-camPos)) > 0.;

		if (lightInView && length((lightPosSS*.5+.5).xy - uv) < .003)
			color = vec4(1.);
	}

	{ // shadow map
		float shadow = getShadow(posV);
		vec3 flashlight = BRDFcookTorrence(posV,camPos,l,mat);
		color.rgb += flashlight*shadow;
		//color = vec4(vec3(shadow),1.);
	}
};
