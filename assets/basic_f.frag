#version 110
varying vec3 color;
uniform float u_time;
uniform vec2 u_resolution;
void main()
{
	gl_FragColor = vec4(color*vec3(u_time), 1.0);
	for (int i = 0; i < 100000; i++) {
		gl_FragColor += mod(gl_FragColor*0.1, 1.0);
	}
};
