#version 410 core

layout (vertices = 3) out;

//TODO in vertex shader is no local to clip space (mul with world-view-proj) necessary
uniform int tessQ;

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	if (gl_InvocationID==0) {
		gl_TessLevelOuter[0] = 16;
		gl_TessLevelOuter[1] = 16;
		gl_TessLevelOuter[2] = 16;
		gl_TessLevelOuter[3] = 16;

		gl_TessLevelInner[0] = 16;
		gl_TessLevelInner[1] = 16;

		//gl_TessLevelOuter[1] = tessQ;
		//gl_TessLevelOuter[3] = tessQ;
	}
}
