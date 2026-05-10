#version 460

uniform float u_time;
uniform vec2 u_resolution;

layout(binding = 0) uniform sampler3D u_tex;

//uniform mat4 u_v; // view
//
//in vec3 posV;
in vec3 colorV;
in vec3 normalV;
//in vec2 uvV;
//in mat3 TBN;
//in mat4 pv;

out vec4 color;

void main() {
	color=vec4(vec3(0.),1.);

	// { // simple draw texture
	//	vec2 uv =gl_FragCoord.xy/u_resolution;
	//	float z = mod(u_time*.2,1.);//(sliceIndex + 0.5) / depth;
	//	vec3 texCoord = vec3(uv,z);
	//	vec4 s = texture(u_tex, texCoord);
	//	color = vec4(s.rgb, 1.0);
	// }

	{
		vec3 lightDir = normalize(vec3(1., -1.0, 1.));
		float diff = max(dot(normalize(normalV), lightDir), 0.0);
		float ambient = 0.2;

		color = vec4(colorV* (diff + ambient),1.);
	}
};
