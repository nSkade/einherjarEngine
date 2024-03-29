#version 460
layout (quads, equal_spacing, ccw) in;

in vec3 colorTCS[];
in vec3 normalTCS[];

out vec3 colorTES;
out vec3 normalTES;

uniform mat4 MVP;
uniform mat4 u_view;

uniform float u_time;
uniform mat4 u_ehjTrans[];

//uniform sampler2D u_noise;
uniform vec2 u_resolution;

#define M_PI 3.14159265358979

#define NUMSTEPS 64
float MINHIT = 0.005;
float MAXDIST = 20.0;
vec3 CAMPOS = vec3(0.0, 1.0, -2.0);
vec3 CAMDIR = vec3(0.0, 0.0, 0.0);
float infinity = 1.0 / 0.0;

mat3 rotationX(float angle) {
	return mat3(	1.0,		0,			0,
			 		0, 	cos(angle),	-sin(angle),
					0, 	sin(angle),	 cos(angle));
}

mat3 rotationY(float angle) {
	return mat3(	cos(angle),		0,		sin(angle),
			 				0,		1.0,			 0,
					-sin(angle),	0,		cos(angle));
}

mat3 rotationZ(float angle) {
	return mat3(	cos(angle),		-sin(angle),	0,
			 		sin(angle),		cos(angle),		0,
							0,				0,		1);
}

struct Def {
	vec3 worldPos;
	vec3 color;
	float depth;
};

struct Mat {
	float dist;
	vec3 color;
	float glow;
};

const int NUMLIGHTS = 2;
vec3 lights[NUMLIGHTS];

float smin(float a, float b, float k) {
	float h = clamp(0.5+0.5*(a-b)/k,0.0,1.0);
	return mix(a,b,h) - k*h*(1.0-h);
}

float rand(float x) {
	return fract(sin(x)*100000.0);
}

float smoothRand(float x) {
	float i = floor(x);
	float f = fract(x);
	return mix(rand(i), rand(i + 1.0), smoothstep(0.,1.,f));
}

float rand(vec2 uv) {
	return rand(dot(uv,vec2(420.69,69.420)));
}

float valueNoise(vec2 uv) {
	vec2 uvi = floor(uv), uvf = fract(uv);
	vec2 u = smoothstep(0.0,1.0,uvf);
	
	// 4 corners
	float a = rand(uvi);
	float b = rand(uvi+vec2(1.0,0.0));
	float c = rand(uvi+vec2(0.0,1.0));
	float d = rand(uvi+vec2(1.0,1.0));
	
	float ab = mix(a,b,u.x);
	float cd = mix(c,d,u.x);
	
	return mix(ab,cd,u.y);
}


vec2 rand2(vec2 uv) {
	vec2 kern = vec2(dot(uv,vec2(232.69,12.420)), dot(uv,vec2(132.49,245.37)));
	return 2.0*fract(sin(kern)*2343.047)-1.0;
}

vec3 rand3(vec3 uv) {
	vec3 kern = vec3(dot(uv,vec3(232.69,12.420,498.54)), dot(uv,vec3(132.49,245.37,85.5673)), dot(uv,vec3(948.04,95.33,5.405)));
	return 2.0*fract(sin(kern)*2343.047)-1.0;
}

float gradientNoise(vec2 uv) {
	vec2 uvi = floor(uv), uvf = fract(uv);
	vec2 u = smoothstep(0.0,1.0,uvf);
	
	//vec2 u = uvf*uvf*uvf;
	//u = ((6*uvf - 15)*uvf + 10)*u;
	
	// 4 corners
	float a = dot(rand2(uvi),uvf);
	float b = dot(rand2(uvi+vec2(1.0,0.0)),uvf-vec2(1.0,0.0));
	float c = dot(rand2(uvi+vec2(0.0,1.0)),uvf-vec2(0.0,1.0));
	float d = dot(rand2(uvi+vec2(1.0,1.0)),uvf-vec2(1.0,1.0));
	
	float ab = mix(a,b,u.x);
	float cd = mix(c,d,u.x);
	
	return mix(ab,cd,u.y);
}

float gradientNoise3(vec3 uv) {
	vec3 uvi = floor(uv), uvf = fract(uv);
	vec3 u = smoothstep(0.0,1.0,uvf);
	
	// 4 corners
	float a = dot(rand3(uvi),uvf);
	float b = dot(rand3(uvi+vec3(1.0,0.0,0.0)),uvf-vec3(1.0,0.0,0.0));
	float c = dot(rand3(uvi+vec3(0.0,1.0,0.0)),uvf-vec3(0.0,1.0,0.0));
	float d = dot(rand3(uvi+vec3(1.0,1.0,0.0)),uvf-vec3(1.0,1.0,0.0));
	
	float ab = mix(a,b,u.x);
	float cd = mix(c,d,u.x);
	
	float lower = mix(ab,cd,u.y);
	
	a = dot(rand3(uvi+vec3(0.0,0.0,1.0)),uvf-vec3(0.0,0.0,1.0));
	b = dot(rand3(uvi+vec3(1.0,0.0,1.0)),uvf-vec3(1.0,0.0,1.0));
	c = dot(rand3(uvi+vec3(0.0,1.0,1.0)),uvf-vec3(0.0,1.0,1.0));
	d = dot(rand3(uvi+vec3(1.0,1.0,1.0)),uvf-vec3(1.0,1.0,1.0));
	
	ab = mix(a,b,u.x);
	cd = mix(c,d,u.x);
	
	float upper = mix(ab,cd,u.y);
	
	return mix(lower,upper,u.z);
}

float d3Noise(in vec3 normal) {
	return gradientNoise3(normal);
	//return gradientNoise(normal.xz)*gradientNoise(normal.xy)*10.0;
	//return valueNoise(normal.xz*scale)*valueNoise(normal.xy*scale);
}

///////////////////////////////////////////////////////////////////////// SCENE SETTINGS
const vec3 scnSpherePos = vec3(0.0,1.0,0.0);

///////////////////////////////////////////////////////////////////////// SCENE FUNCTIONS

float moonNoise(in vec3 normal) {
	normal *= 10.0; // scale normal
	float ret = d3Noise(normal)*0.02;
	//ret += d3Noise(normal*24.0)*0.02;
	//ret += d3Noise(normal*8.0)*0.03;
	//ret += d3Noise(normal*12.0)*0.02;
	
	int steps = 3;
	float multip = 6.0;
	for (int i = 1; i < steps; i++) {
		ret +=d3Noise(normal*multip)*0.02;
		multip *= multip;
	}
	
	return ret;
}

float dstSphere(vec3 p, vec3 c, float r)
{
	//float val = moonNoise(p-c)*0.05;
	return length(p-c)-r;//-val;
}

float dstSphereBump(vec3 p, vec3 c, float r)
{
	float val = moonNoise(p-c)*0.05;
	return length(p-c)-r-val;
}

float distPlane(vec3 p, vec3 n) {
	return dot(p, n)+1.0;
}

float duneFunc(vec3 p) {
	p = p.zyx*0.5;
	vec3 u = 2.0*vec3(gradientNoise(p.xy),0.0,gradientNoise(p.zx));
	return -0.8+0.3*sin(u.x+cos(u.z))-(1.0-abs(sin(u.x+u.z+gradientNoise(u.zx)*0.5)))*0.5;
}

float distDune(vec3 p) {
	vec3 planeN = vec3(0.0,1.0,0.0);
	p.y += 0.2;
	p.xz = p.xz*0.75;
	
	float c = duneFunc(p);
	
	float disp = c;
	return distPlane(p, planeN) + disp;
}

float distFuncNormalDune(vec3 p, vec3 n) {
	vec3 planeN = vec3(0.0,1.0,0.0);
	p.y += 0.2;
	p.xz = p.xz*0.75;
	float c = duneFunc(p);
	
	vec2 stripes = abs(vec2(n.x,n.z));
	vec2 nq = normalize(stripes+1.0);//vec2(stripes.x,stripes.y);
	float off = cos((nq.y*p.x+nq.x*p.z)*500.0);
	
	float opa = max(0.0,(stripes.x+stripes.y)-0.1);
	float s = off;
	
	s = opa * s*0.00175;
	float g = gradientNoise(p.xz*500.0)*0.00175;
	
	return distPlane(p, planeN) + c + s + g;
}

float distFunc(vec3 ray, out vec2 uv) {
	
	vec3 rotRay = ray; //rotationY(u_time)*ray;
	float shape = infinity; //dstSphere(ray, pos, 1.0);
	
	shape = dstSphere(rotRay, scnSpherePos, 1.0);
	shape = min(shape, distDune(ray));
	
	return shape;
}

vec3 normalFunc(vec3 p) {
	vec3 small_step = vec3(0.001,0.0,0.0);
	
	vec2 uv;
	float gradX = distFunc(p + small_step.xyy, uv) - distFunc(p - small_step.xyy, uv);
	float gradY = distFunc(p + small_step.yxy, uv) - distFunc(p - small_step.yxy, uv);
	float gradZ = distFunc(p + small_step.yyx, uv) - distFunc(p - small_step.yyx, uv);
	
	return normalize(vec3(gradX, gradY, gradZ));
}

float distFuncNormal(vec3 ray, out vec2 uv) {
	return dstSphereBump(ray, scnSpherePos, 1.0);
}

vec3 normalFuncMoon(vec3 p) {
	vec3 small_step = vec3(0.003,0.0,0.0);
	
	vec2 uv;
	float gradX = distFuncNormal(p + small_step.xyy, uv) - distFuncNormal(p - small_step.xyy, uv);
	float gradY = distFuncNormal(p + small_step.yxy, uv) - distFuncNormal(p - small_step.yxy, uv);
	float gradZ = distFuncNormal(p + small_step.yyx, uv) - distFuncNormal(p - small_step.yyx, uv);
	
	return normalize(vec3(gradX, gradY, gradZ));
}

vec3 normalFuncDune(vec3 p, vec3 n) {
	vec3 small_step = vec3(0.003,0.0,0.0);
	
	float gradX = distFuncNormalDune(p + small_step.xyy, n) - distFuncNormalDune(p - small_step.xyy, n);
	float gradY = distFuncNormalDune(p + small_step.yxy, n) - distFuncNormalDune(p - small_step.yxy, n);
	float gradZ = distFuncNormalDune(p + small_step.yyx, n) - distFuncNormalDune(p - small_step.yyx, n);
	
	return normalize(vec3(gradX, gradY, gradZ));
}

vec2 explCircle(vec3 pos, vec3 dir, float radius) {
	float a = dot(dir,dir);
	float b = dot(2.0*pos,dir);
	float c = dot(pos,pos)-radius*radius;
	float circle = b*b-4.0*a*c;
	if (circle < 0.0) return vec2(0.0,0.0);
	float fstSol = (-b + sqrt(circle))/(2.0*a);
	return vec2(1.0,fstSol);
}

Def raymarch(vec3 dir, vec3 origin) {
	
	Def def;
	def.depth = infinity;
	
	float dist = 0.0;
	
	// faster convergence by starting at explicit points in space
	// explicit ground at 0.0
	float explG = dir.y < 0.0 ? -origin.y/dir.y : 0.0;
	//explicit circle
	vec2 explC = explCircle(origin-scnSpherePos,dir,1.0);
	origin += dir*(explC.x != 0.0 ? explC.y : 0.0);
	dist = explC.x != 0.0 ? -explC.y : explG;
	
	for (int i = 0; i < NUMSTEPS; i++) {
		vec3 pos = origin + dist * dir;
		
		def.worldPos = pos;
		vec2 uv;
		float closest = distFunc(pos, uv);
			
		if (closest < MINHIT) {
			def.depth = dist;
			vec3 ambient = vec3(1.0);
			vec3 ambientF = vec3(1.0);
			//return normalFunc(pos);
			vec3 light = vec3(5.0,2.0,-6.0);
			vec3 dir_light = normalize(light-pos);
			float dist_light = length(light-pos);
			vec3 normal = normalFunc(pos);
			vec3 oldNormal = normal;
			//def.color = normal;
			//return def;
			
			vec3 magma = vec3(0.0);
			float scale = 0.0;
			// TODO fix shiny border when facing cam? def.color = max(def.color, ambientF*max(0.0,1.0-scale*3.0));
			
			//TODO this is horribly slow do one normal func
			if (pos.y > 0.0) {
				ambient = vec3(20.0/255.0,25.0/255.0,31.5/255.0);
				normal = normalFuncMoon(pos);
				ambientF = normalize(vec3(174.0/255.0,183.0/255.0,198.0/255.0));
				
				oldNormal = oldNormal*(1.25);
				scale = -(dot(dir,oldNormal));
				float glowNoise1 = d3Noise((oldNormal+u_time*0.015)*20.0)-0.28;
				float glowNoise2 = d3Noise((oldNormal+u_time*0.01)*60.0);
				float glowNoise3 = d3Noise((oldNormal+u_time*0.01)*120.0);
				
				vec3 red = vec3(233.0/255.0,48.0/255.0,28.0/255.0);
				vec3 orange = vec3(1.0, 197.0/255.0,73.0/255.0);
				
				vec3 glowColor = max(red*(glowNoise1-0.075),orange*(max(0.0,glowNoise1-0.2)))*2.0;
				glowColor = max(glowColor,(max(0.0,glowNoise1+0.15))*glowNoise2*red*2.0);
				glowColor = max(glowColor,(max(0.0,glowNoise1+0.2))*glowNoise3*red*1.5);
				glowColor *= 5.0;
				magma = (glowColor*scale*scale*scale);
				magma += max(0.0,1.0-scale*3.0)*0.4*(vec3(174.0/255.0,183.0/255.0,198.0/255.0));
			} else {
				ambient = vec3(26.0/255.0,0.28/255.0,40.0/255.0)*0.1;
				ambientF = normalize(vec3(56.0/255.0,63.0/255.0,79.0/255.0));
				float i = dist/MAXDIST;
				i = pow(i,0.2);
				normal = normal*i + (1.0-i)*normalFuncDune(pos,normal);
			}
			float diffuse = max(0.0, dot(normal, dir_light));
			def.color = (ambient) + (ambientF)*diffuse/(dist_light*0.15);
			def.color = pos.y > 0.0 ? def.color + magma : def.color;
			return def;
		}
		if (dist > MAXDIST)
			return def;
		
		dist += closest;
	}
	
	return def;
}

vec4 tellu(vec2 uv) {
	//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	vec2 ndc = uv * 2.0 - vec2(1.0);
	CAMPOS.z -= 1.0;//+sin(u_time*0.5)*2.0;
	CAMPOS.y -= 1.0;
	CAMPOS.z += sin(u_time)*1.25;
	CAMPOS = rotationY(u_time*0.3) * CAMPOS;
	CAMPOS.y += 0.5*sin(u_time*0.4);
	CAMPOS.y = max(0.1,CAMPOS.y);
	vec3 lookat = normalize(scnSpherePos-CAMPOS);
	//lookat = vec3(0.0);
	
	CAMDIR = vec3(ndc+lookat.xy, 1.0);
	CAMDIR = normalize(CAMDIR);
	CAMDIR = rotationY(u_time*0.3) * CAMDIR;
	Def def = raymarch(CAMDIR, CAMPOS);
	float fadeout = MAXDIST-5.0;
	float scale = max(0.0,1.0/log(def.depth-fadeout+5.0));
	//def.color *= scale;
	vec3 bot = vec3(36.0/255.0,45.0/255.0,60.0/255.0);
	vec3 top = vec3(30.0/255.0,34.0/255.0,45.0/255.0);
	def.color += (scale > 0.0 || def.depth > MAXDIST) ? (uv.y*bot+(1.0-uv.y)*top)*(1.0-scale) : vec3(0.0);
	
	vec3 pos = def.worldPos;
	// TODO make colors global
	vec3 red = vec3(233.0/255.0,48.0/255.0,28.0/255.0);
	vec3 orange = vec3(1.0, 197.0/255.0,73.0/255.0);
	
	vec3 Spos = CAMPOS;
	int particleSteps = 6;
	vec3 camToWS = pos-CAMPOS;
	for (int i = 0; i < particleSteps; i++) {
		Spos += (normalize(camToWS)*8.0)/float(particleSteps);
		if (length(Spos-CAMPOS) > length(camToWS))
			break;
		float noise = d3Noise(Spos*30.0)-0.45;
		noise *= 2.0/(length(Spos-scnSpherePos)*10.0-9.0);
		def.color += max(max(0.0,noise)*red, max(0.0,noise-0.1)*orange*1.25)*3.0;
	}
	Spos = CAMPOS;
	//def.color = vec3(0.0);
	for (int i = 0; i < particleSteps; i++) {
		Spos += (normalize(camToWS)*8.0)/float(particleSteps);
		def.color += red*min(1.0,0.065/(exp(pow(length(Spos-scnSpherePos),1.75))));
	}
	vec4 col = vec4(def.color,1.0);
	
	return vec4(col.xyz, 1.0);
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

	//mat3 m = rotationX(p.x+u_time);
	normalTES = triLinterp(normalTCS[0].xyzz,normalTCS[1].xyzz,normalTCS[2].xyzz,normalTCS[3].xyzz,u,v).xyz;
	//normalTES = m*normalTES;


	vec2 uv = p.xy;//gl_FragCoord.xy/u_resolution.xy;
	float aspect = u_resolution.x/u_resolution.y;
	uv.x *= aspect;
	uv.x -= (aspect-1.0)*0.5;
	
	colorTES = tellu(uv).rgb;

	//gl_Position = p;
	gl_Position = MVP * u_view * vec4(p.xyz,1.0);//vec4(gl_TessCoord,1.0);
}