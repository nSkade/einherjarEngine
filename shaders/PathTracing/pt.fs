#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform float u_frame;
uniform vec2 u_resolution;

uniform float u_camChange; // 1 on camera Change

uniform mat4 u_m;
uniform vec3 u_cPos;

uniform sampler2D u_p;

//vec2 explCircle(vec3 pos, vec3 dir, float radius) {
//	float a = dot(dir,dir);
//	float b = dot(2.0*pos,dir);
//	float c = dot(pos,pos)-radius*radius;
//	float circle = b*b-4.0*a*c;
//	if (circle < 0.0) return vec2(0.0,0.0);
//	float fstSol = (-b - sqrt(circle))/(2.0*a);
//	return vec2(1.0,fstSol);
//}

//iq
vec2 sphIntersect( in vec3 ro, in vec3 rd, float ra ) {
	float b	= dot( ro, rd );
	float c	= dot( ro, ro )	- ra*ra;
	float h	= b*b -	c;
	if(	h<0.0 )	return vec2(-1.0); // no intersection
	h =	sqrt( h	);
	return vec2( -b-h, -b+h	);
}

struct Ray {
	vec3 o;
	vec3 d;
};

struct Hit {
	int id;
	float dist;
};

struct Sphere {
	vec3 p;
	float s;
	vec3 col;
	float e; // emission
	float r; // reflection 0 diff, 1 spec
		//TODO seperate color for specular bounce
	vec2 t; // transmission, ior
};

const int sphereCount = 7;
Sphere spheres[sphereCount] = {
	//p           s     col             e  r   t
	{{3.,2.,0.},  1.,   {1.,1.,1.}     ,1.,0.,{0.,0.}},
	//{{3.,2.,0.},  .5,   {1.,1.,1.}     ,0.,0.,{0.,0.}},

	{{0.,0.,0.},  1.,   {0.1,1.,0.1}   ,0.,1.,{0.,0.}},

	{{0.,.5,3.},  .5,   {0.1,0.1,1.}   ,1.,.5,{0.,0.}},
	//{{0.,.5,3.},  .5,   {0.1,0.1,1.}   ,0.,.5,{0.,0.}},

	{{2.,0.,3.},  1.,   {1.,1.,1.}     ,0.,0.,{0.,0.}},
	{{0.,-100.,0.},100., {1.,1.,1.}    ,0.,0.,{0.,0.}},
	{{0.,1.,2.},  .5, {1.,1.,1.}       ,0.,.1,{0.,0.0}},
	{{5.,.75,5.},  1., {1.,1.,1.}       ,0.,.1,{0.,0.0}},

	//{{3.,0.,0.},{2.,2.,2.},{1.,1.,1.}     ,0.,1.},
	//{{0.,1.,0.},{1.,1.,1.},{0.1,1.,0.1}   ,0.,0.},
	//{{0.,1.,3.},{1.,1.,1.},{0.1,0.1,1.}   ,0.,.5},
	//{{2.,1.,3.},{1.,1.,1.},{1.,1.,1.}     ,0.,.9},
	//{{1.,2.5,1.},{100.,1.,100.},{1.,1.,1.},0.,1.},
};

struct Light {
	vec3 p;
};

Light lights[1] = {
	{{1.,2.,1.}},
};

vec3 CAMPOS = vec3(0.);
vec3 CAMDIR = vec3(0.);
vec2 UV = vec2(0.);

Hit traceTrans(Ray r) {
	// get closest hit
	int closestHit = -1;
	float closestHitDist = 1./0.;

	for (int i=0;i<sphereCount;++i) {
		vec3 center = -spheres[i].p;
		float s = spheres[i].s;
		//vec2 hit = explCircle((r.o+center)/s,r.d/s,1.);
		vec3 p=(r.o+center);
		vec3 d= r.d;
		vec2 hit = sphIntersect(p,d,s);

		if (hit.y >= 0. && hit.x >= 0.) {
			if (closestHit == -1 || closestHitDist > hit.x) {
				closestHitDist = hit.x;
				closestHit = i;
				continue;
			}
		} else if (hit.y >= 0.) {
			closestHitDist = hit.y;
			closestHit = i;
			break;
		}
	}

	return Hit(closestHit,closestHitDist);
}

Hit trace(Ray r) {
	// get closest hit
	int closestHit = -1;
	float closestHitDist = 1./0.;

	for (int i=0;i<sphereCount;++i) {
		vec3 center = -spheres[i].p;
		float s = spheres[i].s;
		//vec2 hit = explCircle((r.o+center)/s,r.d/s,1.);
		vec3 p=(r.o+center);
		vec3 d= r.d;
		vec2 hit = sphIntersect(p,d,s);

		if (hit.y >= 0. && hit.x >= 0.) {
			if (closestHit == -1 || closestHitDist > hit.x) {
				closestHitDist = hit.x;
				closestHit = i;
				continue;
			}
		}
	}

	return Hit(closestHit,closestHitDist);
}

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

vec3 rand3(vec3 co) {
	vec3 r = co;
	
	//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	//if (uv.x>.5) { // causes noise to clamp at corners of cube
	//	r.x = rand(r);
	//	r.y = rand(r);
	//	r.z = rand(r);
	//}
	//else { // improves sampling on sphere by sampling closer to the half sphere surface
		r.x = randGauss(r);
		r.y = randGauss(r);
		r.z = randGauss(r);
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

vec3 shade(Ray r,int iter) {
	vec3 c = vec3(1.); // color for absorption
	vec3 e = vec3(0.); // emission

	int bounces = 3;
	Hit h;
	bool anyHit = false;
	for (int i=0;i<bounces;++i) {
		h = trace(r);

		// shade
		if (h.id != -1) {
			anyHit = true;
			vec3 pos = r.o + h.dist * r.d;
			vec3 n = normalize(pos-spheres[h.id].p);

			// translucence
			//if (spheres[h.id].t.x > 0.) {
			//	// entrance
			//	r = Ray(pos,refract(-r.d,n,spheres[h.id].t.y));
			//	h = traceTrans(r);
			//	pos = r.o + h.dist * r.d;
			//	n = normalize(pos-spheres[h.id].p);

			//	// exit
			//	r = Ray(pos,refract(r.d,n,spheres[h.id].t.y));
			//	h = trace(r);
			//	pos = r.o + h.dist * r.d;
			//	n = normalize(pos-spheres[h.id].p);
			//}

			//normal test
			//c = vec3(n*.5+.5);

			//light
			//lc *= dot(n,lights[0].p-pos);
			//c = c*.5 +lc*.5*spheres[h.id].e;

			//vec3 rv = random3(pos+random3(n+hash3(u_time))+hash3(SEED)+hash3(UV.x)+hash3(UV.y));
			
			//vec3 rv = rand3(pos+vec3(uv,n.x));
			//vec3 rv = normalize(rand3(pos+n+vec3(UV,u_frame))*2.-1.);

			//// importance sample, skew towards emissive directions
			//for (int i=0;i<sphereCount;++i) {
			//	if (spheres[i].e > 0.)
			//		rv
			//}

			float w = spheres[h.id].r;
			vec3 nd;
			//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
			//if (uv.x<.5) {
				vec3 rv = normalize(rand3(pos+n+vec3(UV,i)+vec3(iter+u_time)));
				//vec3 rv = normalize(rand3(pos+n+vec3(UV,i)+vec3(iter)));
				nd = normalize(n + rv); // diff // already includes lambert cosine law
			//}
			//else {
			//	vec3 rv = normalize(rand3(pos+n+vec3(UV,iter+u_time))*2.-1.);
			//	nd = rv * sign(dot(n,rv)); // diff // true hemisphere randomness
			//}

			vec3 ns = reflect(r.d,n); // spec
			vec3 nn = nd*(1.-w) + w*ns;
			
			float lc = 0.;
			//if (uv.x<.5)
				lc = 1.;
			//else
			//	lc = dot(n,nn)*2.; // add lambert cosine law to absorption

			int direct=0;

			//vec3 de = vec3(0.);// direct emission				
			//vec3 dc = vec3(1.);// direct color
			//{
			//	// importance sampling, send ray from light sources and calculate brdf contribution of surface towards camera
			//	for (int j=0;j<sphereCount;++j) {
			//		if (spheres[j].e > 0) {
			//			// light source

			//			// test second ray towards light source
			//			Ray r2;
			//			r2.o = pos;
			//			r2.d = normalize(spheres[j].p - pos);// surface towards light
			//			Hit h2 = trace(r2);
			//			
			//			// strength
			//			float s = dot(r2.d,r.d)*.5+.5;

			//			if (h2.id == j) { // no obstruction add light contribution weighted with brdf
			//				vec3 mc2 = spheres[h2.id].col; // mat color
			//				de += c*(mc2*spheres[h2.id].e)*s; //TODO split into emission color and color
			//				dc *= mc2; // absorb other wavelengths
			//				direct +=1;
			//			}
			//		}
			//	}
			//}
			//de /= direct;

			//if (direct==0) {// compute indirect lighting
				vec3 mc = spheres[h.id].col; // mat color
				e += c*(mc*spheres[h.id].e); //TODO split into emission color and color
				c *= mc*lc; // absorb other wavelengths

				//TODO discarding like this doesnt work because energy is just added when a light was hit later on
				//float grayscale = dot(vec3(0.2126,0.7152,0.0722),e);
				//if (grayscale < .0)
				//	break;
			//} else { // average direct lighting
			//	e += de;
			//	c *= dc;
			//}

			r = Ray(pos,nn);
		} else {
			//c *= vec3(1.); e += .5;
			// sun
			float sunDirD = dot(normalize(vec3(1.,1.,1.)),r.d);
			sunDirD = max(0.,pow(sunDirD,100.));
			vec3 sunC = vec3(1.);
			float sunE = 0.;
			e += sunC*sunE*sunDirD; //TODO split into emission color and color
			break;
		}
	}

	// bg color
	if (!anyHit)
		e = vec3(.01);

	return e;
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	//SEED = SEED*hash(u_time*(uv.x)+(uv.y));
	UV = uv;
	//UV.x = mod(UV.x,.5)*2.;
	vec2 ndc = UV * 2.0 - vec2(1.0);

	CAMPOS = u_cPos;
	CAMDIR = vec3(ndc.x,ndc.y,1.);
	CAMDIR = (u_m*normalize(vec4(CAMDIR,1.))).xyz;
	CAMDIR = normalize(CAMDIR);

	Ray r = {CAMPOS,CAMDIR};

#define ACC 1
#if ACC
	int samples = 14;
#else
	//spheres[0].p.x = sin(u_time)*2.;
	//spheres[0].p.z = cos(u_time)*2.;
	//spheres[0].p.y = 4.;
	int samples = 14;
#endif
	vec3 c = vec3(0.);
	for (int i=0;i<samples;++i) {
		c += shade(r,i);
	}
	c /= samples;

	if (u_frame==0)
		color = vec4(c,1.);
	else {
#if ACC
		if (u_camChange == 0.) {
			//color = vec4(c,1.)*.01+.99*texture(u_p,uv);
			//vec4 color2 = max(vec4(c,1.),texture(u_p,uv));
			//color = color*.98+color2*.02;

			float w = 1./(1.+u_frame);
			color = vec4(c,1.)*w + (1.-w)*texture(u_p,uv);

			//color = vec4(c,1.)+texture(u_p,uv);
			color.a = 1.;
		}
		else 
			color = vec4(c,1.)*.5+.5*texture(u_p,uv);
#else
		//color = vec4(c,1.)*.01+.99*texture(u_p,uv);
		color = vec4(c,1.);
#endif
	}
};
