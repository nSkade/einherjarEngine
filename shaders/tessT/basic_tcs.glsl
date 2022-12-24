#version 410 core

// basic tesellation control shader

layout(vertices=3) out;

//TODO in vertex shader is no local to clip space (mul with world-view-proj) necessary

uniform int u_tessQ;

void main() {

	if (gl_InvocationID==0) {
		gl_TessLevelOuter[0] = u_tessQ;
		gl_TessLevelOuter[1] = u_tessQ;
		gl_TessLevelOuter[2] = u_tessQ;
		gl_TessLevelInner[0] = 1;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
