#version 460

// optimal warp sizes are 128 or 256
layout(local_size_x=16,local_size_y=8) in;

layout(binding=0) buffer ssbo {
	float vals[];
};

void main() {
	uvec2 gid = gl_GlobalInvocationID.xy;
	//TODOff use to abstract
	//gl_NumWorkGroups;

	//TODOff check TODOs for other calc pi that uses text buffer
	//unsigned int id = size.x > size.y ? gid.x*size.y+gid.y : gid.y*size.x+gid.x; //TODO this is not needed?
	//unsigned in id = gl_GlobalInvocationID;

	//int intervals = 100000000; //TODOff
	//double delta = 1.0 / double(intervals);
	//float in_val = 0.0;
	//for (unsigned int j = id; j <= intervals; j += size.x*size.y) {
	//	double x = (double(j) - 0.5) * delta;
	//	in_val += float(4.0 / (1.0 + (x*x)));
	//}

	vals[gl_NumWorkGroups.x*gl_WorkGroupSize.x*gid.y + gid.x] = float(gl_NumWorkGroups.x*gl_WorkGroupSize.x*gid.y + gid.x);
}