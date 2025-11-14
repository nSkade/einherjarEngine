#version 460

uniform vec2 u_mouse;
uniform ivec2 u_resolution;
uniform float u_time;
const float PI = 3.14159265359;

//OPTIONS
const float inf=1./0.;
float gridT=0.03;
float scale = 3.;
//OPTIONS

float bounce(float t), tri(float t), bell(float t), pop(float t), tap(float t), pulse(float t), spike(float t), instant(float t), linear(float t), inQuad(float t), outQuad(float t), inOutQuad(float t), outInQuad(float t), inCubic(float t), outCubic(float t), inOutCubic(float t), outInCubic(float t), inQuart(float t), outQuart(float t), inOutQuart(float t), outInQuart(float t), inQuint(float t), outQuint(float t), inOutQuint(float t), outInQuint(float t), inExpo(float t), outExpo(float t), inOutExpo(float t), outInExpo(float t), inCirc(float t), outCirc(float t), inOutCirc(float t), outInCirc(float t), outBounce(float t), inBounce(float t), inOutBounce(float t), outInBounce(float t), inSine(float t), outSine(float t), inOutSine(float t), outInSine(float t), outElastic(float t), inElastic(float t), inOutElastic(float t), outInElastic(float t), inBack(float t), outBack(float t), inOutBack(float t), outInBack(float t), popElastic(float t), tapElastic(float t), pulseElastic(float t), impulse(float t,float damp);

vec3 func(vec2 uv) { float x=uv.x,y=uv.y,f=inf,f2=inf,f3=inf;
	// FUNC HERE
	// FUNC HERE
	// FUNC HERE

	//float b = x;
	//float b2 = mod(x,1./4.)*4.;
	//f=sign(sin(b*4.*PI))*impulse(b2,1.);

	f = pow(x,1./2.2);
	f = sin(x*10.*(1.+sin(u_time)*.5));

	// FUNC HERE
	// FUNC HERE
	// FUNC HERE
	return vec3(f,f2,f3);
}

float grid(vec2 uv);
vec3 plot(vec2 uv);

out vec4 color;

void main () {
	vec2 st = gl_FragCoord.xy/u_resolution.xy;
	st.x *= float(u_resolution.x)/u_resolution.y;
	vec2 uv = st;
	vec3 c=vec3(0.);
	uv = uv*scale-vec2(scale*.5);
	float g = grid(uv);
	c=plot(uv);
	//if (length(c) == 0.)
		c+=vec3(g);
	if (length(uv) < .05)
		c+=vec3(.75);
	color = vec4(c,1.0);
}

float grid(vec2 uv) {
	vec2 g = fract(uv);
	//g=g*g;
	float r = min(g.x,g.y);

	float s=.3;
	if (abs(uv.x) < gridT
		|| abs(uv.y) < gridT)
		s *=3.;
		
	return max(s-smoothstep(0.,gridT,r),0.);
}

vec3 plot(vec2 uv) {
	vec3 v = func(uv);
	//vec3 d=abs(func(vec2(uv.x+0.001,uv.y))-func(vec2(uv.x-0.001,uv.y)));
	//d=vec3(0.);//clamp(d,vec3(0.),vec3(1.));//TODO fix

	const float DX = 0.001;
	const float INV_2DX = 1.0 / (2.0 * DX);

	vec3 dy = func(vec2(uv.x + DX, uv.y)) - func(vec2(uv.x - DX, uv.y));
	vec3 d = abs(dy) * INV_2DX * 0.01;

	if (v.x==inf)
		d.x=(0.);
	if (v.y==inf)
		d.y=(0.);
	if (v.z==inf)
		d.z=(0.);

	vec3 c =vec3(
		1.-smoothstep(0.,gridT+d.x,abs(uv.y-v.x)),
		1.-smoothstep(0.,gridT+d.y,abs(uv.y-v.y)),
		1.-smoothstep(0.,gridT+d.z,abs(uv.y-v.z))
	);
	return c;
}
const float ELASTIC_A = 1.0;
const float ELASTIC_P = 0.3;
const float BACK_A = 1.70158;
const float ELASTIC_DAMP = 1.4;
const float ELASTIC_COUNT = 6.0;

// Elastic Easing (Internal and External)
float outElasticInternal(float t, float a, float p);
float inElasticInternal(float t, float a, float p);
// Back Easing (Internal and External)
float inBackInternal(float t, float a);
float outBackInternal(float t, float a);
// Specialty Elastic Easing (Internal and External)
float popElasticInternal(float t, float damp, float count);
float tapElasticInternal(float t, float damp, float count);
float pulseElasticInternal(float t, float damp, float count);

float bounce(float t) {
	return 4.0 * t * (1.0 - t);
}

float tri(float t) {
	return 1.0 - abs(2.0 * t - 1.0);
}

float inQuint(float t) {
	return pow(t, 5.0);
}

float outQuint(float t) {
	return 1.0 - pow((1.0 - t), 5.0);
}

float inOutQuint(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 * pow(t, 5.0);
	} else {
		return 1.0 - 0.5 * pow((2.0 - t), 5.0);
	}
}

float bell(float t) {
	return inOutQuint(tri(t));
}

float pop(float t) {
	return 3.5 * (1.0 - t) * (1.0 - t) * sqrt(t);
}

float tap(float t) {
	return 3.5 * t * t * sqrt(1.0 - t);
}

float pulse(float t) {
	return t < 0.5 ? tap(t * 2.0) : -pop(t * 2.0 - 1.0);
}

float spike(float t) {
	return exp(-10.0 * abs(2.0 * t - 1.0));
}

float instant(float t) {
	return 1.0;
}

float linear(float t) {
	return t;
}

// --- Power Easing Functions ---

float inQuad(float t) {
	return t * t;
}
float outQuad(float t) {
	return -t * (t - 2.0);
}
float inOutQuad(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 * t * t;
	} else {
		return 1.0 - 0.5 * pow((2.0 - t), 2.0);
	}
}
float outInQuad(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 - 0.5 * pow((1.0 - t), 2.0);
	} else {
		return 0.5 + 0.5 * pow((t - 1.0), 2.0);
	}
}

float inCubic(float t) {
	return t * t * t;
}
float outCubic(float t) {
	return 1.0 - pow((1.0 - t), 3.0);
}
float inOutCubic(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 * pow(t, 3.0);
	} else {
		return 1.0 - 0.5 * pow((2.0 - t), 3.0);
	}
}
float outInCubic(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 - 0.5 * pow((1.0 - t), 3.0);
	} else {
		return 0.5 + 0.5 * pow((t - 1.0), 3.0);
	}
}

float inQuart(float t) {
	return pow(t, 4.0);
}
float outQuart(float t) {
	return 1.0 - pow((1.0 - t), 4.0);
}
float inOutQuart(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 * pow(t, 4.0);
	} else {
		return 1.0 - 0.5 * pow((2.0 - t), 4.0);
	}
}
float outInQuart(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 - 0.5 * pow((1.0 - t), 4.0);
	} else {
		return 0.5 + 0.5 * pow((t - 1.0), 4.0);
	}
}

float inQuint(float t); // Defined earlier for 'bell'
float outQuint(float t); // Defined earlier for 'bell'
float inOutQuint(float t); // Defined earlier for 'bell'

float outInQuint(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 - 0.5 * pow((1.0 - t), 5.0);
	} else {
		return 0.5 + 0.5 * pow((t - 1.0), 5.0);
	}
}

// --- Exponential Easing Functions ---

float inExpo(float t) {
	return pow(1000.0, t - 1.0) - 0.001;
}
float outExpo(float t) {
	return 1.001 - pow(1000.0, -t);
}
float inOutExpo(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 * pow(1000.0, t - 1.0) - 0.0005;
	} else {
		return 1.0005 - 0.5 * pow(1000.0, 1.0 - t);
	}
}
float outInExpo(float t) {
	if (t < 0.5) {
		return outExpo(t * 2.0) * 0.5;
	} else {
		return inExpo(t * 2.0 - 1.0) * 0.5 + 0.5;
	}
}

// --- Circular Easing Functions ---

float inCirc(float t) {
	return 1.0 - sqrt(1.0 - t * t);
}
float outCirc(float t) {
	return sqrt(-t * t + 2.0 * t);
}
float inOutCirc(float t) {
	t = t * 2.0;
	if (t < 1.0) {
		return 0.5 - 0.5 * sqrt(1.0 - t * t);
	} else {
		t = t - 2.0;
		return 0.5 + 0.5 * sqrt(1.0 - t * t);
	}
}
float outInCirc(float t) {
	if (t < 0.5) {
		return outCirc(t * 2.0) * 0.5;
	} else {
		return inCirc(t * 2.0 - 1.0) * 0.5 + 0.5;
	}
}

// --- Bounce Easing Functions ---

float outBounce(float t) {
	if (t < 1.0 / 2.75) {
		return 7.5625 * t * t;
	} else if (t < 2.0 / 2.75) {
		t = t - 1.5 / 2.75;
		return 7.5625 * t * t + 0.75;
	} else if (t < 2.5 / 2.75) {
		t = t - 2.25 / 2.75;
		return 7.5625 * t * t + 0.9375;
	} else {
		t = t - 2.625 / 2.75;
		return 7.5625 * t * t + 0.984375;
	}
}
float inBounce(float t) {
	return 1.0 - outBounce(1.0 - t);
}
float inOutBounce(float t) {
	if (t < 0.5) {
		return inBounce(t * 2.0) * 0.5;
	} else {
		return outBounce(t * 2.0 - 1.0) * 0.5 + 0.5;
	}
}
float outInBounce(float t) {
	if (t < 0.5) {
		return outBounce(t * 2.0) * 0.5;
	} else {
		return inBounce(t * 2.0 - 1.0) * 0.5 + 0.5;
	}
}

// --- Sine Easing Functions ---

float inSine(float t) {
	return 1.0 - cos(t * (PI * 0.5));
}
float outSine(float t) {
	return sin(t * (PI * 0.5));
}
float inOutSine(float t) {
	return 0.5 - 0.5 * cos(t * PI);
}
float outInSine(float t) {
	if (t < 0.5) {
		return outSine(t * 2.0) * 0.5;
	} else {
		return inSine(t * 2.0 - 1.0) * 0.5 + 0.5;
	}
}

// --- Elastic Easing Functions (Internal and External) ---

float outElasticInternal(float t, float a, float p) {
	return a * pow(2.0, -10.0 * t) * sin((t - p / (2.0 * PI) * asin(1.0 / a)) * 2.0 * PI / p) + 1.0;
}
float inElasticInternal(float t, float a, float p) {
	return 1.0 - outElasticInternal(1.0 - t, a, p);
}

float outElastic(float t) {
	return outElasticInternal(t, ELASTIC_A, ELASTIC_P);
}
float inElastic(float t) {
	return inElasticInternal(t, ELASTIC_A, ELASTIC_P);
}
float inOutElastic(float t) {
	return t < 0.5 ? 0.5 * inElasticInternal(t * 2.0, ELASTIC_A, ELASTIC_P) : 0.5 + 0.5 * outElasticInternal(t * 2.0 - 1.0, ELASTIC_A, ELASTIC_P);
}
float outInElastic(float t) {
	return t < 0.5 ? 0.5 * outElasticInternal(t * 2.0, ELASTIC_A, ELASTIC_P) : 0.5 + 0.5 * inElasticInternal(t * 2.0 - 1.0, ELASTIC_A, ELASTIC_P);
}

// --- Back Easing Functions (Internal and External) ---

float inBackInternal(float t, float a) {
	return t * t * (a * t + t - a);
}
float outBackInternal(float t, float a) {
	t = t - 1.0;
	return t * t * ((a + 1.0) * t + a) + 1.0;
}

float inBack(float t) {
	return inBackInternal(t, BACK_A);
}
float outBack(float t) {
	return outBackInternal(t, BACK_A);
}
float inOutBack(float t) {
	return t < 0.5 ? 0.5 * inBackInternal(t * 2.0, BACK_A) : 0.5 + 0.5 * outBackInternal(t * 2.0 - 1.0, BACK_A);
}
float outInBack(float t) {
	return t < 0.5 ? 0.5 * outBackInternal(t * 2.0, BACK_A) : 0.5 + 0.5 * inBackInternal(t * 2.0 - 1.0, BACK_A);
}

// --- Specialty Elastic Functions (Internal and External) ---

float popElasticInternal(float t, float damp, float count) {
	return (pow(1000.0, -pow(t, damp)) - 0.001) * sin(count * PI * t);
}
float tapElasticInternal(float t, float damp, float count) {
	return (pow(1000.0, -pow((1.0 - t), damp)) - 0.001) * sin(count * PI * (1.0 - t));
}
float pulseElasticInternal(float t, float damp, float count) {
	return t < 0.5 ? tapElasticInternal(t * 2.0, damp, count) : -popElasticInternal(t * 2.0 - 1.0, damp, count);
}

float popElastic(float t) {
	return popElasticInternal(t, ELASTIC_DAMP, ELASTIC_COUNT);
}
float tapElastic(float t) {
	return tapElasticInternal(t, ELASTIC_DAMP, ELASTIC_COUNT);
}
float pulseElastic(float t) {
	return pulseElasticInternal(t, ELASTIC_DAMP, ELASTIC_COUNT);
}

// --- Impulse Function ---

float impulse(float t,float damp) {
	t = pow(t, damp);
	return t * (pow(1000.0, -t) - 0.001) * 18.6;
}
