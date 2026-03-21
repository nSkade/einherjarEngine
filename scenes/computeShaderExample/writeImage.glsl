#version 460

//TODOf dynamic size scaling
// dynamicCPShader = dynamicCPShader +
// "#version 430\n"
// "layout(local_size_x=" + std::to_string(computeShaderDims[0]) + ",local_size_y=" + std::to_string(computeShaderDims[1]) + ",local_size_z=1) in;\n"
// "layout(r32ui, binding=0) uniform uimage2D valueImage;\n"
// "void main()\n"
// "{\n"
// "	uvec2 gid = gl_GlobalInvocationID.xy;\n"
// "	uint in_val = imageLoad(valueImage, ivec2(gid)).x;\n"
// "	imageStore(valueImage, ivec2(gid), uvec4(in_val+1, 0,0,0));\n"
// "}\n";

layout(rgba32f, binding=0) uniform writeonly image2D outImg;

uniform float u_time;

// optimal warp sizes are 128 or 256
layout(local_size_x=16,local_size_y=8) in;

void main() {
	ivec2 id = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(id)/imageSize(outImg);
	vec4 c=vec4(uv,sin(u_time),1.);
	imageStore(outImg,id,c);
}
