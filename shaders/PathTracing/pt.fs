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

// http://glslsandbox.com/e#61476.1
float hash(	float n	){
	return fract(sin(n)*1751.5453);
}

float hash1( vec2 p	){
	return fract(sin(p.x+131.1*p.y)*1751.5453);
}

vec3 hash3(	float n	){
	return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(43758.5453123,22578.1459123,19642.3490423));
}

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

const int sphereCount = 6;
Sphere spheres[sphereCount] = {
	{{3.,2.,0.},  2.,   {1.,1.,1.}     ,3.,0.,{0.,0.}},
	{{0.,0.,0.},  1.,   {0.1,1.,0.1}   ,0.,1.,{0.,0.}},
	{{0.,.5,3.},  .5,   {0.1,0.1,1.}   ,2.,.5,{0.,0.}},
	{{2.,0.,3.},  1.,   {1.,1.,1.}     ,0.,0.,{0.,0.}},
	{{0.,-100.,0.},100., {1.,1.,1.}    ,0.,0.,{0.,0.}},
	{{0.,1.,2.},  .5, {1.,1.,1.}       ,0.,.4,{0.,0.03}},

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

float SEED = 38912.319823;
float rand() {
	SEED = hash(SEED);
	return SEED;
}
vec3 rand3() {
	vec3 r;
	r.x = rand()*2.-1.;
	r.y = rand()*2.-1.;
	r.z = rand()*2.-1.;
	return normalize(r);
}

vec3 shade(Ray r) {
	vec3 c = vec3(1.);
	vec3 e = vec3(0.);

	int steps = 3;
	Hit h;
	bool anyHit = false;
	for (int i=0;i<steps;++i) {
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
			vec3 rv = rand3();
			rv = rv * sign(dot(n,rv));

			float w = spheres[h.id].r;
			vec3 nd = normalize(n + rv); // diff
			vec3 ns = reflect(r.d,n); // spec
			vec3 nn = nd*(1.-w) + w*ns;

			vec3 mc = spheres[h.id].col; // mat color
			e += c*(mc*spheres[h.id].e); //TODO split into emission color and color
			c *= mc;// // absorb other wavelengths

			r = Ray(pos,nn);
		} else {
			//c *= vec3(1.); e += .5;
			break;
		}
	}
	if (!anyHit)
		e = vec3(.1);

	return e;
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	SEED = SEED*hash(u_time*(uv.x)+(uv.y));
	UV = uv;
	vec2 ndc = uv * 2.0 - vec2(1.0);

	CAMPOS = u_cPos;
	CAMDIR = vec3(ndc.x,ndc.y,1.);
	CAMDIR = (u_m*normalize(vec4(CAMDIR,1.))).xyz;
	CAMDIR = normalize(CAMDIR);

	Ray r = {CAMPOS,CAMDIR};

#define ACC 0
#if ACC
	int samples = 1;
#else
	spheres[0].p.x = sin(u_time)*2.;
	spheres[0].p.z = cos(u_time)*2.;
	spheres[0].p.y = 4.;
	int samples = 14;
#endif
	vec3 c = vec3(0.);
	for (int i=0;i<samples;++i) {
		c += shade(r);
	}
	c /= samples;

#if ACC
	if (u_camChange == 0.) {
		//color = vec4(c,1.)*.01+.99*texture(u_p,uv);
		//vec4 color2 = max(vec4(c,1.),texture(u_p,uv));
		//color = color*.98+color2*.02;

		float w = 1./(1.+u_frame);
		color = vec4(c,1.)*w + (1.-w)*texture(u_p,uv);
	}
	else 
		color = vec4(c,1.)*.5+.5*texture(u_p,uv);
#else
	color = vec4(c,1.)*.2+.8*texture(u_p,uv);
#endif
};
