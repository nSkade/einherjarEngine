#version 410 core
// basic tesellation evaluation shader

layout (triangles, equal_spacing, ccw) in;

void main() {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;

	vec4 p = u*p0+v*p1+w*p2;

	gl_Position = p;
}