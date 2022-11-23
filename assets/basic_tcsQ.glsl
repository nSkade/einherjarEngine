#version 410 core

// basic tesellation control shader

layout(vertices=4) out;
//TODO in vertex shader is no local to clip space (mul with world-view-proj) necessary

uniform int u_tessQ;

in vec3 colorV[];
in vec3 normalV[];

out vec3 colorTCS[];
out vec3 normalTCS[];

void main() {
	colorTCS[gl_InvocationID] = colorV[gl_InvocationID];
	normalTCS[gl_InvocationID] = normalV[gl_InvocationID];

	if (gl_InvocationID==0) {
		gl_TessLevelOuter[0] = u_tessQ;
		gl_TessLevelOuter[1] = 1;
		gl_TessLevelOuter[2] = u_tessQ;
		gl_TessLevelOuter[3] = 1;

		gl_TessLevelInner[0] = 1;
		gl_TessLevelInner[1] = u_tessQ;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
