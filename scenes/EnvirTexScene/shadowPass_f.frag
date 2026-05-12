#version 460

//out vec4 color;

layout(binding = 0) uniform sampler2D u_albedo;

in vec2 uvV;

void main() {
	float alpha = texture(u_albedo,uvV).a;
	if (alpha < 0.1)
		discard;
};
