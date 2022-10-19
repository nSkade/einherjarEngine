
uniform sampler2D u_noise;
uniform vec2 u_resolution;
uniform float u_time;

#define M_PI 3.14159265358979

#define NUMSTEPS 64
#define PARTICLESTEPS 4
#define MOONNOISELAYERS 2
float MINHIT = 0.01;
float MAXDIST = 26.0;
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
	//return valueNoise(normal.xz)*valueNoise(normal.xy);
}

float moonNoise(in vec3 normal) {
	normal *= 10.0; // scale normal
	float ret = d3Noise(normal)*0.02;
	//ret += d3Noise(normal*24.0)*0.02;
	//ret += d3Noise(normal*8.0)*0.03;
	//ret += d3Noise(normal*12.0)*0.02;
	
	float multip = 6.0;
	for (int i = 1; i < MOONNOISELAYERS; i++) {
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

float distDune(vec3 p) {
	vec3 planeN = vec3(0.0,1.0,0.0);
	
	float disp;// = .3*sin(.4*p.z)*sin(p.x+gradientNoise(p.zx))*2.5;
	
	disp = 0.3*sin(1.0*p.x+cos(p.z*1.3))-(1.0-abs(sin(p.x+valueNoise(p.zx))))*0.4;
	return distPlane(p, planeN) + disp;
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
	uv = vec2(rotRay.x,rotRay.y*0.5);
	float shape = infinity; //dstSphere(ray, pos, 1.0);
	//shape = min(shape, dstSphere(ray, vec3(-5.0,5.0, 2.0), 0.5));
	//shape = dstSphere(ray, vec3(-0.0,sin(u_time)*1.2, -3.0), 0.1);
	
	vec3 spherePos = vec3(-0.0,1.3, -0.0);
	shape = dstSphere(rotRay, spherePos, 1.0);
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
	
	vec3 spherePos = vec3(-0.0,1.3, -0.0);
	return dstSphereBump(ray, spherePos, 1.0);
}

vec3 normalFuncMoon(vec3 p) {
	vec3 small_step = vec3(0.003,0.0,0.0);
	
	vec2 uv;
	float gradX = distFuncNormal(p + small_step.xyy, uv) - distFuncNormal(p - small_step.xyy, uv);
	float gradY = distFuncNormal(p + small_step.yxy, uv) - distFuncNormal(p - small_step.yxy, uv);
	float gradZ = distFuncNormal(p + small_step.yyx, uv) - distFuncNormal(p - small_step.yyx, uv);
	
	return normalize(vec3(gradX, gradY, gradZ));
}

Def raymarch(vec3 dir, vec3 origin) {
	
	Def def;
	def.depth = infinity;
	
	float dist = 0.0;
	for (int i = 0; i < NUMSTEPS; i++) {
		vec3 pos = origin + dist * dir;
		
		def.worldPos = pos;
		vec2 uv;
		float closest = distFunc(pos, uv);
			
		if (closest < MINHIT) {
			def.depth = dist;
			vec3 ambient = vec3(33.0/255.0,35.0/255.0,47.0/255.0);
			ambient = vec3(25.0/255.0,25.0/255.0,32.0/255.0);
			//return normalFunc(pos);
			vec3 light = vec3(5.0,2.0,-6.0);
			vec3 dir_light = normalize(light-pos);
			float dist_light = length(light-pos);
			vec3 normal = normalFunc(pos);
			vec3 oldNormal = normal;
			//def.color = normal;
			//return def;
			if (pos.y > -0.1) {
				//ambient = vec3(0.1);
				normal = normalFuncMoon(pos);
			} else {
				//ambient = vec3(0.05);
			}
			float diffuse = max(0.0, dot(normal, dir_light));
			vec3 tex = vec3(1.0); //texture(u_noise, normal);
			vec4 col = vec4(ambient*tex,1.0) + vec4(tex,0.0)*diffuse/(dist_light*0.2);
		//	if (pos.y > -0.1) {
		//		col += vec4(vec3(moonNoise(normal)),1.0);
		//	}
			//vec4 col = vec4(ambient,1.0) + vec4(1.0,1.0,1.0,0.0)*diffuse;
			def.color = col.xyz;
			if (pos.y > -0.1) {
				//def.color = vec3(0.0);
				vec3 camdir = inverse(rotationY(u_time*0.3)) * oldNormal;
				vec3 screenNormal = camdir;//cross(oldNormal, camdir);
				//screenNormal = camdir;
				float scale = -(dot(dir,oldNormal));
				float glowNoise1 = d3Noise((oldNormal+u_time*0.015)*20.0)-0.18;
				float glowNoise2 = d3Noise((oldNormal+u_time*0.01)*60.0);
				float glowNoise3 = d3Noise((oldNormal+u_time*0.01)*120.0);
				
				vec3 red = vec3(233.0/255.0,48.0/255.0,28.0/255.0);
				vec3 orange = vec3(1.0, 197.0/255.0,73.0/255.0);
				
				vec3 glowColor = max(red*(glowNoise1-0.075),orange*(max(0.0,glowNoise1-0.2)))*2.0;
				glowColor = max(glowColor,(max(0.0,glowNoise1+0.15))*glowNoise2*red*2.0);
				glowColor = max(glowColor,(max(0.0,glowNoise1+0.2))*glowNoise3*red*1.5);
				glowColor *= 5.0;
				def.color += (glowColor*scale*scale*scale);
			}
			return def;
		}
		if (dist > MAXDIST)
			return def;
		
		dist += closest;
	}
	
	return def;
}

void main(void)
{
	vec2 uv = viewIndepUV();
	//vec2 uv = gl_FragCoord.xy/u_resolution.xy;
	vec2 ndc = uv * 2.0 - vec2(1.0);
	u_time *= 1.0;
	CAMPOS.z -= 1.0;
	CAMPOS.z += sin(u_time);
	CAMPOS = rotationY(u_time*0.3) * CAMPOS;
	CAMPOS.y += sin(u_time*0.3);
	vec3 lookat = normalize(vec3(0.0,1.3,0.0)-CAMPOS);
	lookat = vec3(0.0);
	
	CAMDIR = vec3(ndc, 1.0)+lookat;
	CAMDIR = normalize(CAMDIR);
	CAMDIR = rotationY(u_time*0.3) * CAMDIR;
	Def def = raymarch(CAMDIR, CAMPOS);
	float fadeout = 25.0;
	if (def.depth > fadeout) {
		float scale = 1.0/log(def.depth-fadeout+3.0);
		def.color *= scale;
		vec3 bot = vec3(36.0/255.0,45.0/255.0,60.0/255.0);
		vec3 top = vec3(30.0/255.0,34.0/255.0,45.0/255.0);
		def.color += (uv.y*bot+(1.0-uv.y)*top)*(1.0-scale);
	}
	
	vec3 pos = def.worldPos;
	// TODO make colors global
	vec3 red = vec3(233.0/255.0,48.0/255.0,28.0/255.0);
	vec3 orange = vec3(1.0, 197.0/255.0,73.0/255.0);
	
	vec3 spherePos = vec3(-0.0,1.3, -0.0);
	vec3 Spos = CAMPOS;
	vec3 camToWS = pos-CAMPOS;
	for (int i = 0; i < PARTICLESTEPS; i++) {
		Spos += (normalize(camToWS)*8.0)/PARTICLESTEPS;
		if (length(Spos-CAMPOS) > length(camToWS))
			break;
		float noise = d3Noise(Spos*30.0)-0.45;
		noise *= 2.0/(length(Spos-spherePos)*10.0-9.0);
		def.color += max(max(0.0,noise)*red, max(0.0,noise-0.1)*orange*1.25)*3.0;
	}
	Spos = CAMPOS;
	//def.color = vec3(0.0);
	for (int i = 0; i < PARTICLESTEPS; i++) {
		Spos += (normalize(camToWS)*8.0)/PARTICLESTEPS;
		def.color += red*min(1.0,0.085/(exp(pow(length(Spos-spherePos),1.75))));
	}
	vec4 col = vec4(def.color,1.0);
	
	gl_FragColor = vec4(col.xyz, 1.0);
}