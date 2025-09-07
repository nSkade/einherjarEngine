#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform int u_mbd;
uniform vec2 u_mouse;

uniform float u_pencilSize;
uniform vec4 u_pencilColor;

float sdCircle( vec2 p, float r )
{
	return length(p) - r;
}

vec4 pencil(vec2 uv, out bool inside) {
	vec4 ret = vec4(0.);
	float radiusSquared = u_pencilSize;
	float ratio = u_resolution.x/u_resolution.y;
	vec2 uva=uv; uva.x *= ratio;
	vec2 mouse = u_mouse/u_resolution; mouse.x *= ratio;
	inside = false;
	if (sdCircle(uva-mouse,radiusSquared) <= 0.) {
		ret = u_pencilColor;
		inside = true;
	}
	return ret;
}

void main() {
	if (u_frame < 2) {
		color = vec4(0.);
		return;
	}
	vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	if (u_mbd==1) {
		bool inside = false;
		vec4 colorPencil = pencil(uv,inside);
		if (inside) {
			color = colorPencil;
		} else
			discard;
	} else
		discard;
};
