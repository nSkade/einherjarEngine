#version 460
precision mediump float;
#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY__PRESET 12

#define FXAA_GREEN_AS_LUMA 1

uniform vec2 u_resolution;

in vec3 colorV;
in vec3 normalV;

out vec4 color;

#include "shaders/Fxaa3_11.h"

precision highp float;

uniform sampler2D uSourceTex;
uniform vec2 RCPFrame;
uniform bool u_enableFXAA;

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	if (!u_enableFXAA) {
		vec4 c = texture(uSourceTex,uv);
		color = vec4(c.xyz,1.0);
		return;
	}

	color = FxaaPixelShader(uv,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),	//// FxaaFloat4 fxaaConsolePosPos,
		uSourceTex,							// FxaaTex tex,
		uSourceTex,							//// FxaaTex texLuma,
		uSourceTex,							//// FxaaTex fxaaConsole360TexExpBiasNegOne,
		uSourceTex,							//// FxaaTex fxaaConsole360TexExpBiasNegTwo,
		RCPFrame,							// FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),	//// FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),	//// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),	//// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f,								// FxaaFloat fxaaQualitySubpix,
		0.166f,								// FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f,							// FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f,								//// FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f,								//// FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f,								//// FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)	//// FxaaFloat fxaaConsole360ConstDir,
	);
	return;
};
