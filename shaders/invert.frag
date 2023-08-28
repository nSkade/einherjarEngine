#version 460

uniform bool u_enableFSR;
uniform vec2 u_texRes;
uniform vec2 u_resolution;
uniform sampler2D u_Tex;

in vec3 colorV;
in vec3 normalV;

out vec4 color;

#define A_GPU 1
#define A_GLSL 1
#include "lib/ffx-fsr/ffx_a.h"
#define FSR_EASU_F 1
AU4 con0, con1, con2, con3;

AF4 FsrEasuRF(AF2 p) { return textureGather(u_Tex, p, 0); }
AF4 FsrEasuGF(AF2 p) { return textureGather(u_Tex, p, 1); }
AF4 FsrEasuBF(AF2 p) { return textureGather(u_Tex, p, 2); }

#include "lib/ffx-fsr/ffx_fsr1.h"

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	if (!u_enableFSR) {
		vec3 c = texture(u_Tex,uv).xyz;
		color = vec4(c,0.0);
		return;
	}
	
	FsrEasuCon(con0, con1, con2, con3,
		u_texRes.x, u_texRes.y,  // Viewport size (top left aligned) in the input image which is to be scaled.
		u_texRes.x, u_texRes.y,  // The size of the input image.
		u_resolution.x, u_resolution.y); // The output resolution.
	
	AU2 gxy = AU2(gl_FragCoord); // Integer pixel position in output.
	AF3 Gamma2Color = AF3(0, 0, 0);
	FsrEasuF(Gamma2Color, gxy, con0, con1, con2, con3);	
	vec3 c = Gamma2Color;
	color = vec4(c,0.0);
}
