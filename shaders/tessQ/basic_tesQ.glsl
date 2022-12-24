#version 410 core
// basic tesellation evaluation shader

layout (quads, equal_spacing, ccw) in;

in vec3 colorTCS[];
in vec3 normalTCS[];

out vec3 colorTES;
out vec3 normalTES;

uniform mat4 MVP;
uniform mat4 u_view;

uniform float u_time;
uniform mat4 u_ehjTrans[];

mat3 rotationX(float angle) {
	return mat3(	1.0,		0,			0,
					0,	cos(angle),	-sin(angle),
					0,	sin(angle),	 cos(angle));
}

vec4 triLinterp(vec4 p0, vec4 p1, vec4 p2, vec4 p3, float u, float v) {
	vec4 p4 = (p3 - p0) * u + p0;
	vec4 p5 = (p2 - p1) * u + p1;
	return (p5 - p4) * v + p4;
}

void main() {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec4 p3 = gl_in[3].gl_Position;


	//vec4 p = u*p0+v*p1+w*p2;
	vec4 p = triLinterp(p0,p1,p2,p3,u,v);
	//p = u_ehjTrans[gl_PrimitiveID]*p;

	mat3 m = rotationX(p.x+u_time);
	normalTES = triLinterp(normalTCS[0].xyzz,normalTCS[1].xyzz,normalTCS[2].xyzz,normalTCS[3].xyzz,u,v).xyz;
	normalTES = m*normalTES;

	colorTES = vec3(1.0f);

	//gl_Position = p;
	gl_Position = MVP * u_view * vec4(p.xyz,1.0);//vec4(gl_TessCoord,1.0);
	
}