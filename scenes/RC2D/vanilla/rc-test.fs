#version 460
// ref https://jason.today/rc

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform int u_mbd;
uniform vec2 u_mouse;

uniform int u_baseRayCount;
uniform int u_raySteps;
uniform float u_rayNoise;
uniform float u_rayDist;
uniform float u_lightStr;

uniform sampler2D u_tex;
uniform sampler2D u_texJumpFlood;
uniform sampler2D u_texPrev;

uniform int u_viewPass;
uniform int u_cascade;
uniform int u_cascadeCount;
uniform int u_viewCascade;

uniform float u_overlap; // start / end intervall overlap

bool outOfBounds(vec2 uv) {
	return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

float rand(vec2 co){
	return fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 raymarch(vec2 uv) {
	if (u_viewCascade != 0)
		if (u_cascade != u_viewCascade-1)
			return texture(u_texPrev,uv);
	
	//bool lastLayer = u_cascade == u_cascadeCount;
	float ratio = u_resolution.x/u_resolution.y;
	vec2 coord = floor(uv * u_resolution);

	//uv = lastLayer ? uv : floor(uv*u_resolution/2.)*2./u_resolution;

	//uv = floor(uv*u_resolution/pow(2.,u_cascade))*pow(2.,u_cascade)/u_resolution;

	//float startD = float(pow(u_baseRayCount,u_cascade))  *u_rayDist;
	//float endD   = float(pow(u_baseRayCount,u_cascade+1))*u_rayDist;
	
	float maxLen = length(vec2(1.,1.));
	//float maxLen = length(vec2(ratio,1.)); //TODO
	int base = u_baseRayCount; //TODO
	
	//float startD = float(pow(base,u_cascade  )-1.)/(pow(base,u_cascadeCount)-1.) *maxLen*u_rayDist;
	//float endD   = float(pow(base,u_cascade+1)-1.)/(pow(base,u_cascadeCount)-1.) *maxLen*u_rayDist;

	float minRes = min(u_resolution.x,u_resolution.y);
	 // max len to avoid having to use one extra cascade but huge quality loss
	float startD = float(pow(base,u_cascade  )) / minRes / u_rayDist;// * maxLen;
	float endD   = float(pow(base,u_cascade+1)) / minRes / u_rayDist;// * maxLen;

	if (u_cascade > 1) {
		startD -= u_overlap;
		endD += u_overlap;
	}
	//startD=0.; // for unloading lower level cascades we march from the center of the probe

	//if (!lastLayer) endD = length(vec2(ratio,1.)); // maximum dist possible on screenspace

	//float rayCount = u_baseRayCount;
	float rayCount = pow(u_baseRayCount,u_cascade+1);
	
	//TODOff abstract to use arbitrary ray Count not just pow(4,x)
	//float nearestQuad = pow(4,ceil(log2(rayCount) / 2.0));
	//float nearestQuadBase = pow(4,ceil(log2(u_baseRayCount) / 2.0));
	
	float probeSqrtBase = sqrt(float(u_baseRayCount));
	//float probeSpacing = rayCount == u_baseRayCount ? 1.0 : probeSqrtBase;
	float probeSpacing = sqrt(float(rayCount/u_baseRayCount));
	vec2  probeSize = floor(u_resolution / probeSpacing);
	vec2  probeRelativePosition = mod(coord, probeSize);

	vec2  rayPos = floor(coord/ probeSize);
	float baseIndex = float(u_baseRayCount) * (rayPos.x + (probeSpacing * rayPos.y));
	
	vec2  probeCenter = (probeRelativePosition + 0.5) * probeSpacing;
	vec2  probeCenterNormalized = probeCenter / u_resolution;
	
	float oneOverRayCount = 1.0 / float(rayCount);
	float tauOverRayCount = 3.141592 * 2. * oneOverRayCount;
	float angleStepSize = tauOverRayCount; //TODO
	
	vec4 radiance = vec4(0.0);
	
	// Distinct random value for every pixel, note proper RC doesnt use noise
	//float noise = rand(uv + vec2(u_time))*u_rayNoise;

	for(int i = 0; i < u_baseRayCount; i++) { // note only shoot base ray count, baseIndex ofsets in texture
		vec4 e = vec4(0.); // emission
		float index = baseIndex + float(i);
		float angleStep = index + .5;
		float angle = angleStepSize * angleStep;

		//float angle = tauOverRayCount * float(i) + u_cascade*tauOverRayCount*.5;
		vec2 rayDirectionUv = vec2(cos(angle), -sin(angle));
		rayDirectionUv = normalize(rayDirectionUv);

		vec2 sampleUv = probeCenterNormalized + rayDirectionUv*startD * vec2(1./ratio,1.);
		//vec2 sampleUv = uv + rayDirectionUv*startD;
		float distTotal = startD;
		
		for (int j = 0; j< u_raySteps; j++) {
			vec2 nearestSeed = texture(u_texJumpFlood, sampleUv).xy;
			float dist = distance(sampleUv, nearestSeed);
			
			//TODO(without sdf)
			//float dist = (endD-startD)/u_raySteps;

			distTotal += dist;
			if (distTotal > endD)
				break;
			
			sampleUv += rayDirectionUv * dist * vec2(1./ratio,1.);
			if (outOfBounds(sampleUv))
				break;
			
			if (dist <= 0.0001) { //TODO improve using pixel dist
				vec4 s = texture(u_tex, sampleUv);
				e += vec4(s.rgb * u_lightStr,s.a) / (1.+distTotal);
				break;
			}
		}

		// merge step with previous cascade from texture (recursively)
		bool nonOpaque = e.a == 0.0;
		if (nonOpaque) {
			float upperSpacing = pow(probeSqrtBase, u_cascade + 1);
			vec2 upperSize = floor(u_resolution / upperSpacing);
			vec2 upperPos = vec2(mod(index, upperSpacing),
			                     floor(index / upperSpacing)) * upperSize;

			vec2 offset = (probeRelativePosition + .5) / probeSqrtBase;
			offset = clamp(offset,vec2(.5),upperSize-.5);

			vec4 upperSample = texture(u_texPrev,(upperPos + offset) / u_resolution);

			e += vec4(upperSample.rgb, upperSample.a);
		}

		radiance += e;
	}

	//if (u_cascade==0)
		radiance.rgb /= u_baseRayCount; // radiance falloff

	//TODO test change ray brightness with noise uniform for now
	//if (u_cascade==0)
		radiance.rgb *= 1.+9.*(1.-u_rayNoise);
	return radiance;
	//return vec4(radiance * oneOverRayCount,1.);
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	
	if (u_viewPass==1) {
		color = texture(u_texJumpFlood,uv);
		return;
	}

	if (u_viewCascade == 0 && u_cascade == 0) {
		vec4 light = texture(u_tex, uv);
		if (light.a > 0.1)
			color = light;
		else
			color = raymarch(uv);
		// srgb
		//TODO srgb introduces heavy banding on dark areas, they get too bright too quickly, find fix, GL_RGBA32F doesnt fix this
		//color.rgb = pow(color.rgb,vec3(1.0/2.2));
		color.rgb = pow(color.rgb,vec3(1.0/1.6));
		//color.rgb = pow(color.rgb,vec3(.25+.75*1.0/2.2));
	} else
		color = raymarch(uv);
};
