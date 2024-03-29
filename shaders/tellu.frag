
uniform vec2 u_resolution;
uniform float u_time;

uniform vec3 u_cPos;
uniform vec3 u_cDir;
uniform vec3 u_cUp;
uniform vec3 u_cRgt;
uniform float u_cFoc;

#define M_PI 3.14159265358979

#define NUMSTEPS 16//32 //25
float MINHIT = 0.005; //0.04;
float MAXDIST = 40.0; //TODO 20.0;
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
	float disp = duneFunc(p);
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

vec2 viewIndepUV() {
	vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	float aspect = u_resolution.x/u_resolution.y;
	uv.x *= aspect;
	uv.x -= (aspect-1.0)*0.5;
	return uv;
}

float distFunc(vec3 ray, out vec2 uv) {
	
	vec3 rotRay = ray; //rotationY(u_time)*ray;
	float shape = infinity; //dstSphere(ray, pos, 1.0);
	
	shape = dstSphere(rotRay, scnSpherePos, 1.0);
	shape = min(shape, distDune(ray));
	
	return shape;
}

vec3 normalFunc(vec3 p) {
	vec3 small_step = vec3(0.01,0.0,0.0);
	
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
	float fstSol = (-b - sqrt(circle))/(2.0*a);
	return vec2(1.0,fstSol);
}

vec2 explPlane(vec3 pos, vec3 dir, vec3 n, vec3 a) {
	// dot(p-a,n)=0
	// p=pos+dir*t;
	// dot(pos+dir*t-a,n)=0
	// dot(pos-a,n) + dot(dir*t,n) = 0
	// dot(dir*t,n) = -dot(pos-a,n)
	// t = -dot(pos-a,n)/dot(dir,n)
	float t = -dot(pos-a,n)/dot(dir,n);
	if (t<0.0) //nohit
		return vec2(0.0,0.0);
	else
		return vec2(1.0,t);
}

vec2 explPlaneCurved(vec3 pos, vec3 dir, vec3 n, vec3 a) {
	// dot(p-a,n)=0
	// p=pos+dir*t;
	// dot(pos+dir*t-a,n)=0
	// dot(pos-a,n) + dot(dir*t,n) = 0
	// dot(dir*t,n) = -dot(pos-a,n)
	// t = -dot(pos-a,n)/dot(dir,n)
	float t = -dot(pos-a,n)/dot(dir,n);
	vec3 p = pos+dir*t;
	float lp = 0.0;//length(p)*0.4;
	if (t<0.0) //nohit
		return vec2(0.0,0.0);
	else
		return vec2(1.0,t-lp);
}

float g = 0.0;

Def raymarch(vec3 dir, vec3 origin) {
	
	Def def;
	def.depth = infinity;
	
	float dist = 0.0;
	
	// faster convergence by starting at explicit points in space
	// explicit ground at 0.0
	float explG = dir.y < 0.0 ? -origin.y/dir.y : 0.0;
	//explicit circle
	vec2 explC = explCircle(origin-scnSpherePos,dir,1.0);
	dist = explC.x != 0.0 ? explC.y : explG;
	dist = explC.y < 0.0 ? explG : dist;
	
	vec3 pos = vec3(0.);
	for (int i = 0; i < NUMSTEPS; i++) {
		pos = origin + dist * dir;
		
		vec2 uv;
		float closest = distFunc(pos, uv);
		
		//vec3 llp = pos;
		//g += 0.4/(llp.z*llp.z+0.1)*0.01;
		//g += 0.4/(llp.x*llp.x+0.1)*0.01;
		
		if (closest < MINHIT) {
			def.worldPos = pos;
			def.depth = dist;
			return def;
		}
		if (dist > MAXDIST) {
			def.worldPos = pos;
			return def;
		}
		
		dist += closest;
	}
	def.worldPos = pos;
	def.depth = pos.y > 0. ? infinity : dist;
	return def;
}

const int NUMLIGHTS = 2;
vec3 lights[NUMLIGHTS];

vec3 shade(Def def, vec3 dir) {
	vec3 c=vec3(0.);
	if (def.depth == infinity)
		return c;
	
	vec3 pos = def.worldPos;
	float dist = def.depth;
	
	vec3 ambient = vec3(1.0);
	vec3 ambientF = vec3(1.0);
	//return normalFunc(pos);
	vec3 normal = normalFunc(pos);
	vec3 oldNormal = normal;
	//c = normal;
	//return c;

	vec3 magma = vec3(0.0);
	float scale = 0.0;
	// TODO fix shiny border when facing cam? def.color = max(def.color, ambientF*max(0.0,1.0-scale*3.0));
	
	//TODO this is horribly slow do one normal func
	if (pos.y > 0.0) {
		ambient = vec3(20.0/255.0,25.0/255.0,31.5/255.0);
		normal = normalFuncMoon(pos);
		ambientF = normalize(vec3(174.0/255.0,183.0/255.0,198.0/255.0));
		
		//oldNormal = oldNormal*(1.25);
		scale = -(dot(dir,oldNormal));
		
		// use 2d screenspace normals?
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
	
	lights[0] = vec3(5.0,3.0,-6.0);
	lights[1] = vec3(-5.0,1.5,6.0);
	
	vec3 diffuse = vec3(0.);
	
	for (int i=0;i<NUMLIGHTS;++i) {
		vec3 l = lights[i], c = normalize(CAMDIR);
		//lights[i].xz = l.xz*c.xz;
		
		vec3 dir_light = normalize(lights[i]-pos);
		float dist_light = length(lights[i]-pos);
		float fd = dot(CAMDIR,normalize(l))*.5+.5;
		fd = fd*1.1-.1;
		float id = fd*max(0.0, dot(normal, dir_light));//*(1.-float(i)*(sin(u_time)*0.5+0.5));
		diffuse += 1.5*ambientF*id/(dist_light*.15);
	}
	ambient *= ambient;

	c = (ambient) + diffuse;//TODO make light for moon brighter but keep dunes dark 
	c = pos.y > 0.0 ? c + magma : c;
	
	return c;
}

float lightlane(float depth, vec3 pn, vec3 off, float wid, float llo, float str) {
	vec2 px = explPlaneCurved(CAMPOS, CAMDIR, pn,off);
	float dp = px.y;
	float g;
	if (dp > 0.0 && dp < depth) {
		vec3 llp = CAMPOS+CAMDIR*dp;
		vec3 f = llp;//cross(llp,CAMDIR);
		float lle = f.y-0.5-0.3*sin(dot(llp,cross(pn,vec3(0.0,1.0,0.0)))*.5+u_time+llo);
		float fade = abs(dot(CAMDIR,pn))*min(1.0,dp)*str;
		g += 0.7/(lle*lle*500.0*wid+1.0)*fade*fade;
	}
	return g;
}

void main(void)
{
	vec2 uv = viewIndepUV();
	//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	vec2 ndc = uv * 2.0 - vec2(1.0);
	//u_time *= 1.0;
	//CAMPOS.z -= 1.0;//+sin(u_time*0.5)*2.0;
	//CAMPOS.y -= 1.0;
	//CAMPOS.z += sin(u_time)*1.25;
	//CAMPOS = rotationY(u_time*0.3) * CAMPOS;
	////CAMPOS = rotationY(3.14)*CAMPOS;
	////CAMPOS = rotationY(u_mouse.x/u_resolution.x*3.14*2.0)*CAMPOS;
	////CAMPOS = rotationZ(u_mouse.y/u_resolution.y*3.14)*CAMPOS;
	//CAMPOS.y += 0.5*sin(u_time*0.4);
	//CAMPOS.y = max(0.1,CAMPOS.y);
	//vec3 lookat = normalize(scnSpherePos-CAMPOS);
	////lookat = vec3(0.0);
	//vec3 camdiro = lookat;
	//CAMDIR = vec3(ndc+lookat, 1.0);

	CAMPOS = u_cPos;
	CAMDIR = normalize(-u_cRgt*ndc.x+u_cUp*ndc.y+u_cDir*u_cFoc);
	float abrTime = u_time*3.0;
	if (mod(abrTime,3.14*4.0) < 3.14) {
		float abre = sin(abrTime)*0.01;
		abre = mod(float(gl_FragCoord.x),2.0) < 1.0 ? -abre : abre;
		//CAMDIR.x += abre;
	}

	CAMDIR = normalize(CAMDIR);
	//CAMDIR = rotationY(u_mouse.x/u_resolution.x*3.14*2.0)*CAMDIR;
	//CAMDIR = rotationY(u_time*0.3) * CAMDIR;
	Def def = raymarch(CAMDIR, CAMPOS);
	//gl_FragColor = vec4(def.worldPos, 1.0);
	//return;
	def.color = shade(def,CAMDIR);

	float fadeout = MAXDIST;
	fadeout =log(def.depth-fadeout+13.0);
	float scale = clamp(log(fadeout),0.0,1.0);
	//def.color *= scale;
	vec3 bot = vec3(36.0/255.0,45.0/255.0,60.0/255.0);
	vec3 top = vec3(30.0/255.0,34.0/255.0,45.0/255.0);
	vec3 bg = (uv.y*top+(1.0-uv.y)*bot);
	//def.color += (scale > 0.0 || def.depth > MAXDIST) ? (uv.y*bot+(1.0-uv.y)*top)*(1.0-scale) : vec3(0.0);
	//scale = 1.0-scale;
	def.color = scale*bg+(1.0-scale)*def.color;
	
	vec3 pos = def.worldPos;
	// TODO make colors global
	vec3 red = vec3(233.0/255.0,48.0/255.0,28.0/255.0);
	vec3 orange = vec3(1.0, 197.0/255.0,73.0/255.0);
	
	vec3 Spos = CAMPOS;
	int particleSteps = 6;
	vec3 camToWS = pos-CAMPOS;
	//def.color = vec3(0.0);
	for (int i = 0; i < particleSteps; i++) {
		//glow
		Spos += (normalize(camToWS)*8.0)/float(particleSteps);
		def.color += 2.0*red*min(1.0,0.065/(exp(pow(length(Spos-scnSpherePos),1.75))));
		
		// particles
		if (length(Spos-CAMPOS) < length(camToWS)) {
			float noise = d3Noise(Spos*30.0+u_time*1.0)-0.45;
			//float noise = gradientNoise(Spos.xy*30.0)-0.45; 
			noise *= 2.0/(length(Spos-scnSpherePos)*10.0-9.0);
			def.color += max(max(0.0,noise)*red, max(0.0,noise-0.1)*orange*1.25)*3.0;
		}
	}
	vec4 col = vec4(def.color,1.0);
	
	float ll = 0.;
	ll += lightlane(def.depth, vec3(1.0,0.0,0.0), vec3(-2.0,0.0,0.0),1.0,0.0,1.0);
	ll += lightlane(def.depth, normalize(vec3(.9,.0,.1)), vec3(-2.0,0.0,0.0),100.0,0.3*3.14,0.6);
	ll += lightlane(def.depth, normalize(vec3(.3,.0,.7)), vec3(0.0,0.0,-3.0),10.0,0.6*3.14,1.0);

	gl_FragColor = vec4(col.xyz+vec3(1.0,0.2,0.2)*(g+ll)*5.0, 1.0);
}
