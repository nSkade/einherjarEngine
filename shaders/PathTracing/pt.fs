#version 460

in vec3 colorV;
in vec3 normalV;

out vec4 color;

uniform float u_time;
uniform vec2 u_resolution;

uniform mat4 u_m;
uniform vec3 u_cPos;

struct Ray {
	vec3 o;
	vec3 d;
};

vec2 explCircle(vec3 pos, vec3 dir, float radius) {
	float a = dot(dir,dir);
	float b = dot(2.0*pos,dir);
	float c = dot(pos,pos)-radius*radius;
	float circle = b*b-4.0*a*c;
	if (circle < 0.0) return vec2(0.0,0.0);
	float fstSol = (-b - sqrt(circle))/(2.0*a);
	return vec2(1.0,fstSol);
}

//TODO
struct Sphere {
	vec3 p;
	vec3 s;
};

Sphere spheres[3] = {
	{{3.,0.,0.},{2.,2.,2.}},
	{{0.,1.,0.},{1.,1.,1.}},
	{{1.,1.,1.},{1.,1.,1.}},
};

vec3 CAMPOS = vec3(0.);
vec3 CAMDIR = vec3(0.);

vec3 trace() {
	vec3 c = vec3(0.);

	// get closest hit
	int closestHit = -1;
	float closestHitDist = 1./0.;

	Ray r = {CAMPOS,CAMDIR};

	for (int i=0;i<3;++i) {
		vec3 center = spheres[i].p;
		vec3 s = spheres[i].s;
		vec2 hit = explCircle((r.o+center)/s,r.d/s,1.);
		if (hit.x != 0. && hit.y > 0.) {
			if (closestHit == -1 || closestHitDist > hit.y) {
				closestHitDist = hit.y;
				closestHit = i;
				continue;
			}
		}
	}
	if (closestHit != -1) {
		vec3 pos = CAMPOS + closestHitDist*r.d;
		vec3 n = normalize(pos+spheres[closestHit].p);
		c = vec3(n*.5+.5);
	}

	return c;
}

void main() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy; //texVp/u_resolution;
	vec2 ndc = uv * 2.0 - vec2(1.0);

	CAMPOS = u_cPos;
	CAMDIR = vec3(ndc.x,ndc.y,1.);
	CAMDIR = (u_m*normalize(vec4(CAMDIR,1.))).xyz;
	CAMDIR = normalize(CAMDIR);

	color = vec4(trace(),1.);
};
