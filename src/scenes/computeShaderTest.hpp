#include "sceneUtils.hpp"

//TODO remove obsolete includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

class ComputeShaderTestScene : IScene {
public:
	void setup() {

	}

	// inputs from compute shader
	//
	// in uvec3 gl_NumWorkGroups;
	// in uvec3 gl_WorkGroupID;
	// in uvec3 gl_LocalInvocationID;
	// in uvec3 gl_GlobalInvocationID;
	// in uint gl_LocalInvocationIndex;
	//
	// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Inputs

	// outputs will need to be either in a shader storage buffer object
	// or an image load store
	//
	// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Outputs
	unsigned int computeShaderDims[3] = {1000,1000,1};

	// static const char* compute_shader_text =
	// "#version 430\n"
	// "layout(local_size_x=1,local_size_y=1,local_size_z=1) in;\n"
	// "layout(r32f, binding=0) uniform image2D valueImage;\n"
	// "void main()\n"
	// "{\n"
	// "	uvec2 gid = gl_GlobalInvocationID.xy;\n"
	// "	float in_val = imageLoad(valueImage, ivec2(gid)).x;\n"
	// "	imageStore(valueImage, ivec2(gid), vec4(in_val+1.0,0.0,0.0,0.0));\n"
	// "}\n";
	
	// int j;
	// int intervals = 10;
	// double delta, x, pi = 0.0;
	
	// delta = 1.0 / (double)intervals;
	
	// for (j = 0; j <= intervals; j++)
	// {
		// x = ((double) j - 0.5) * delta;
		// pi += 4.0 / (1.0 + (x * x));
	// }
	// printf ("π = %.10f\n", pi * delta);
	static constexpr char* compute_shader_text =
	"#version 430\n"
	"layout(local_size_x=1,local_size_y=1,local_size_z=1) in;\n"
	"layout(r32f, binding=0) uniform image2D valueImage;\n"
	"void main()\n"
	"{\n"
	"	uvec2 gid = gl_GlobalInvocationID.xy;\n"
	"	ivec2 size = imageSize(valueImage);\n"
	"	unsigned int id = size.x > size.y ? gid.x*size.y+gid.y : gid.y*size.x+gid.x;\n"
	"	int intervals = 100000000;\n"
	"	double delta = 1.0 / double(intervals);\n"
	"	float in_val = 0.0;\n"
	"	for (unsigned int j = id; j <= intervals; j += size.x*size.y) {\n"
	"		double x = (double(j) - 0.5) * delta;\n"
	"		in_val += float(4.0 / (1.0 + (x*x)));\n"
	"	}\n"
	//"	float in_val = imageLoad(valueImage, ivec2(gid)).x;\n"
	"	imageStore(valueImage, ivec2(gid), vec4(in_val,0.0,0.0,0.0));\n"
	"}\n";
	
	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	};
	
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	};

	int run() {
	
		GLFWwindow* window;
		GLuint vertex_buffer, vertex_shader, fragment_shader, program;
		GLint mvp_location, vpos_location, vcol_location;

		glfwSetErrorCallback(error_callback);

		if (!glfwInit())
			exit(EXIT_FAILURE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetKeyCallback(window, key_callback);

		glfwMakeContextCurrent(window);

		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glfwSwapInterval(1);

		unsigned int err = 0;
		err = glGetError();
		while ((err != 0)) {
			std::cout << err << "\n";
			err = glGetError();
		}
		// compute shader

		// compile shader
		unsigned int shader;
		unsigned int id;
		unsigned int out_tex;

		// int maxSizeX;
		// int maxSizeY;
		// int maxSizeZ;

		// glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxSizeX);
		// glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxSizeY);
		// glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxSizeZ);
		// std::cout << "max dims: x: " << maxSizeX << " y: " << maxSizeY << " y: " << maxSizeZ << "\n";
		// computeShaderDims[0] = maxSizeX;
		// computeShaderDims[1] = maxSizeY;

		// std::cout << "max dims: x: " << computeShaderDims[0] << " y: " << computeShaderDims[1] << "\n";

		// std::string dynamicCPShader = "";

		// std::cout << "itoa0: " << std::to_string(computeShaderDims[0]) << "\n";
		// std::cout << "itoa1: " << std::to_string(computeShaderDims[1]) << "\n";

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

		// std::cout << dynamicCPShader;

		// char* dynamicCPShaderC = (char*) malloc(sizeof(char)*(dynamicCPShader.size()+1));
		// memcpy(dynamicCPShaderC, dynamicCPShader.c_str(), (dynamicCPShader.size()+1));

		// std::cout << dynamicCPShaderC;

		shader = glCreateShader( GL_COMPUTE_SHADER );
		glShaderSource( shader, 1, &compute_shader_text, NULL );
		glCompileShader( shader );

		// create program
		id = glCreateProgram();
		glAttachShader( id, shader );
		glLinkProgram( id );

		// cleanup
		glDeleteShader( shader );
		err = glGetError();
		while ((err != 0)) {
			std::cout << err << "\n";
			err = glGetError();
			return -1;
		}
		// create input/output textures
		glGenTextures( 1, &out_tex );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, out_tex );

		// turns out we need this. huh.
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

		// int j;
		// int intervals = 1000000000;
		// double delta, x, pi = 0.0;

		// delta = 1.0 / (double)intervals;

		// for (j = 0; j <= intervals; j++)
		// {
			// x = ((double) j - 0.5) * delta;
			// pi += 4.0 / (1.0 + (x * x));
		// }
		// printf ("π = %.10f\n", pi * delta);

		float* values = (float*) calloc(computeShaderDims[0]*computeShaderDims[1],sizeof(float));
		for (uint32_t i = 0; i < computeShaderDims[0]*computeShaderDims[1]; i++) {
			values[i] = (float) i;
		}

		glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, computeShaderDims[0], computeShaderDims[1], 0, GL_RED, GL_FLOAT, values );
		glBindImageTexture( 0, out_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F );

		glUseProgram( id );
		err = glGetError();
		while ((err != 0)) {
			std::cout << err << "\n";
			err = glGetError();
			return -1;
		}
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, out_tex );


		glDispatchCompute( computeShaderDims[0], computeShaderDims[1], 1 );
		double x = 0.0;
		//while (x < 10.0) {
		//	clock_t t;
		//	t = clock();
		//	t = clock() - t;
		//	x += ((double)t)/CLOCKS_PER_SEC;
		//}
		//return -1;
		//glMemoryBarrier( GL_ALL_BARRIER_BITS );

		unsigned int collection_size = computeShaderDims[0] * computeShaderDims[1];
		std::vector<float> compute_data( collection_size );
		glGetTexImage( GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data() );

		double pi = 0.0;
		double intervals = 100000000.0;
		for (uint32_t i = 0; i < compute_data.size(); i++) {
			//std::cout << compute_data[i]*1.0 / intervals << " ";
			pi += compute_data[i];
		}
		std::cout << "\n";
		std::cout << pi*1.0 / intervals << "\n";

		glDeleteProgram( id );
		// while render

		glfwDestroyWindow(window);

		glfwTerminate();
		exit(EXIT_SUCCESS);
		return 0;
	}
};
