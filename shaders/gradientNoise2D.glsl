vec2 rand2(vec2 uv) {
	vec2 kern = vec2(dot(uv,vec2(232.69,12.420)), dot(uv,vec2(132.49,245.37)));
	return 2.0*fract(sin(kern)*2343.047)-1.0;
}

float gradientNoise(vec2 uv) {
	vec2 uvi = floor(uv), uvf = fract(uv);
	vec2 u = smoothstep(0.0,1.0,uvf);
	
	// 4 corners
	float a = dot(rand2(uvi),uvf);
	float b = dot(rand2(uvi+vec2(1.0,0.0)),uvf-vec2(1.0,0.0));
	float c = dot(rand2(uvi+vec2(0.0,1.0)),uvf-vec2(0.0,1.0));
	float d = dot(rand2(uvi+vec2(1.0,1.0)),uvf-vec2(1.0,1.0));
	
	float ab = mix(a,b,u.x);
	float cd = mix(c,d,u.x);
	
	return mix(ab,cd,u.y);
}
