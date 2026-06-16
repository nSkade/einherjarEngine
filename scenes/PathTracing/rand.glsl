////////////////////////////////////////////
// random
////////////////////////////////////////////
// http://glslsandbox.com/e#61476.1
//float hash(	float n	){
//	return fract(sin(n)*1751.5453);
//}
//float hash1( vec2 p	){
//	return fract(sin(p.x+131.1*p.y)*1751.5453);
//}
//vec3 hash3(	float n	){
//	return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(43758.5453123,22578.1459123,19642.3490423));
//}
vec3 random3(vec3 c) {
	float	j =	4096.0*sin(dot(c,vec3(17.0,	59.4, 15.0)));
	vec3 r;
	r.z =	fract(512.0*j);
	j	*= .125;
	r.x =	fract(512.0*j);
	j	*= .125;
	r.y =	fract(512.0*j);
	return r-0.5;
}

//float SEED = 38912.319823;
//float rand(float o) {
//	SEED = hash(SEED+o+u_frame);
//	return SEED;
//}

float rand(float co){
	return fract(sin(co*12.9898) * 43758.5453);
}
float rand(vec2 co){
	return fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec3 co) {
	return fract(sin(dot(co ,vec3(12.9898,78.233,193.92385))) * 43758.5453);
}
float randGauss(vec3 co) { // gaussian distr, random
	float t = 2. * 3.1415926 * rand(co);
	float r = sqrt(-2. * log(rand(co+t)));
	return r * cos(t);
}
float randGauss(float co) { // gaussian distr, random
	float t = 2. * 3.1415926 * rand(co);
	float r = sqrt(-2. * log(rand(co+t)));
	return r * cos(t);
}

/**
 * @brief equally distributed on sphere unlike rand3(
*/
vec3 rand3Gauss(vec3 co) {
	vec3 r = co;
	
	//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	//if (uv.x>.5) { // causes noise to clamp at corners of cube
	//	r.x = rand(r);
	//	r.y = rand(r);
	//	r.z = rand(r);
	//}
	//else { // improves sampling on sphere by sampling closer to the half sphere surface
		r.x = randGauss(r);
		r.y = randGauss(r + vec3(1.23, 4.56, 7.89));
		r.z = randGauss(r + vec3(9.87, 6.54, 3.21));

		// add extra offset, not needed
		//r.x = randGauss(r);
		//r.y = randGauss(r + vec3(1.23, 4.56, 7.89));
		//r.z = randGauss(r + vec3(9.87, 6.54, 3.21));
	//}
	
	//SEED = noise1(o+SEED+u_frame);
	//r = noise3(o+SEED+u_frame);
	//r = noise3(o+u_time);
	return r;
}

//
//vec3 rand3(vec3 o) {
//	vec3 r;
//	//r.x = rand(o.y)*2.-1.;
//	//r.y = rand(o.z)*2.-1.;
//	//r.z = rand(o.x)*2.-1.;
//	//SEED = noise1(o+SEED+u_frame);
//	//r = noise3(o+SEED+u_frame);
//	r = noise3(o+u_time);
//	return normalize(r);
//}
////////////////////////////////////////////
// random
////////////////////////////////////////////
