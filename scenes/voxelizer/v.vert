#version 460
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNrm;
layout(location = 2) in vec3 vCol;
layout(location = 3) in vec2 vUV;

#include "DrawCommandIndirect.glsl"

layout(std430, binding = 1) buffer InstanceBuffer {
	EHJSD_Instance instances[];
};

uniform mat4 u_pvm;
uniform ivec3 u_volumeDim;

out vec3 colorV;
out vec3 normalV;

void main() {
	EHJSD_Instance data = instances[gl_InstanceID];
	
	//TODO pos .w not needed was from posscale
	// pos.xyz = world/uv position, .w = scale
	vec3 worldPos = vPos * data.pos.w + data.pos.xyz * u_volumeDim;
	
	colorV= data.col.xyz;
	normalV= vNrm;
	gl_Position = u_pvm * vec4(worldPos, 1.0);
}