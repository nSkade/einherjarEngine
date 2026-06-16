#include "../src/suOGL.hpp"
#include "GAPI/OGL/GLUtils.hpp"
#include "suCMN.hpp"

//TODO remove obsolete includes
//TODO pch, remove, #include <GLFW/glfw3.h>
#include <vector>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define SCENETYPE ComputeShaderTestScene
class ComputeShaderTestScene : IScene {
public:
	void setup() {

	}

	// inputs from compute shader
	//
	// in uvec3 gl_NumWorkGroups;
	// in uvec3	gl_WorkGroupSize; // (the one defined with layout)
	// in uvec3 gl_WorkGroupID;
	// in uvec3 gl_LocalInvocationID;
	// in uvec3 gl_GlobalInvocationID; // = (gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID)
	// in uint gl_LocalInvocationIndex; // = (gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x)
	//
	// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Inputs

	// outputs will need to be either in a shader storage buffer object
	// or an image load store
	//
	// more details at https://www.khronos.org/opengl/wiki/Compute_Shader#Outputs
	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	};
	
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	};

	void example1_testPi() {
		unsigned int computeShaderDims[3] = {10000,10000,1};
		int intervals = computeShaderDims[0]*computeShaderDims[1];

		GLProgram glp;
		glp.addSourceFromFile(EHJ_THIS_FOLDER()+"computePi.glsl",GL_COMPUTE_SHADER);
		if (0) { // valid
			int j;
			double delta, x, pi = 0.0;

			delta = 1.0 / (double)intervals;

			for (j = 0; j <= intervals; j++)
			{
				x = ((double) j - 0.5) * delta;
				pi += 4.0 / (1.0 + (x * x));
			}
			printf ("π = %.10f\n", pi * delta);
		}

		glp.createProgram();
		glp.bind();
		////TODO remove,,, ehj_gl_err();

		//TODO use ssbo instead
#if 0
		// create input/output textures
		unsigned int out_tex;
		glGenTextures(1, &out_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, out_tex);

		// turns out we need this. huh.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else //GLTexture
		GLTexture::Opt gltOpt;
		gltOpt.internalformat=GL_R32F;
		gltOpt.width=computeShaderDims[0];
		gltOpt.height=computeShaderDims[1];
		gltOpt.texturefilter=GL_NEAREST;
		GLTexture glt(gltOpt);
#endif

		std::vector<float> values(computeShaderDims[0]*computeShaderDims[1]);
		for (int i=0;i<values.size();++i)
			values[i]=i;
		glBindTexture(GL_TEXTURE_2D,glt.getTex());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, computeShaderDims[0], computeShaderDims[1], 0, GL_RED, GL_FLOAT, values.data());
		//glActiveTexture(GL_TEXTURE0); // this is active by default
#if 0
		//glBindTexture(GL_TEXTURE_2D, out_tex);
		glBindImageTexture(0, out_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
#else //GLTexture
		glBindImageTexture(0, glt.getTex(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
#endif

		glDispatchCompute(computeShaderDims[0], computeShaderDims[1], 1);

		std::vector<float> compute_data(intervals);

		// glGetTexImage does not require memory barrier, it contains implicit sync
		//glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data());

		double pi = 0.0;
		for (uint32_t i = 0; i < compute_data.size(); i++) {
			//std::cout << compute_data[i]*1.0 / intervals << " ";
			pi += compute_data[i];
		}
		std::cout << "\n";
		std::cout << pi*1.0 / intervals << "\n";
	}

	void example2_testImg(GLFWwindow* window) {
		unsigned int computeShaderDims[3] = {500,500,1};
		int w=computeShaderDims[0];
		int h=computeShaderDims[1];
		glfwSetWindowSize(window,w,h);
		GLProgram glp;
		glp.addSourceFromFile(EHJ_THIS_FOLDER()+"writeImage.glsl",GL_COMPUTE_SHADER);

		glp.createProgram();
		glp.bind();

		GLFrameBuffer::Opt fbOpt;
		fbOpt.internalformat=GL_RGBA32F;
		fbOpt.res=ivec2(w,h);
		fbOpt.texturefilter=GL_NEAREST;
		GLFrameBuffer fb(fbOpt);
		
		glBindImageTexture(0, fb.getTexCol(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		ehj::Clock clock;
		while (!glfwWindowShouldClose(window)) {
			clock.update();
			static float time = 0.;
			time += clock.getDeltaTime();
			glUniform1f(glp.getUnfLoc("u_time"),time);

			ivec3 ls; // local work group size
			glGetProgramiv(glp.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);

			// work group size (grid size)
			int wgsx= (w+ls[0]-1) / ls[0];
			int wgsy= (h+ls[1]-1) / ls[1];
			
			glDispatchCompute(wgsx,wgsy,1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER,fb.getFBO());
			glBlitFramebuffer(0,0,w,h,0,0,w,h,GL_COLOR_BUFFER_BIT,GL_NEAREST);
			
			glfwSwapBuffers(window);
			glfwPollEvents();
			
			if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);
		}
	}

	void example3_ssbo() {
		unsigned int computeShaderDims[3] = {10,20,1};
		int intervals = computeShaderDims[0]*computeShaderDims[1];

		GLProgram glp;
		glp.addSourceFromFile(EHJ_THIS_FOLDER()+"ssboTest.glsl",GL_COMPUTE_SHADER);

		glp.createProgram();
		glp.bind();

		std::vector<float> values(computeShaderDims[0]*computeShaderDims[1]);
		for (int i=0;i<values.size();++i)
			values[i]=0;

		GLuint ssbo;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, values.size() * sizeof(float), values.data(), GL_STATIC_DRAW);
		//TODO glBufferData deprecated, use glNamedBufferStorage instead for fixed size data
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); // binding point 0

		//glBindBuffer(GL_SHADER_STORAGE_BUFFER,0);//TODO need to rebind, what if we want to bind multiple ssbo?

		ivec3 ls; // local work group size
		glGetProgramiv(glp.getID(), GL_COMPUTE_WORK_GROUP_SIZE, &ls[0]);

		// work group size (grid size)
		int wgsx= (computeShaderDims[0]+ls[0]-1) / ls[0];
		int wgsy= (computeShaderDims[1]+ls[1]-1) / ls[1];
		
		glDispatchCompute(wgsx,wgsy,1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		float* compute_data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(values.data(),compute_data,values.size()*sizeof(float));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		double pi = 0.0;
		for (uint32_t i = 0; i < values.size(); i++) {
			std::cout << values[i] << "\n";
			//pi += values[i];
		}
		std::cout << "\n";
		//std::cout << pi*1.0 / intervals << "\n";
	}

	int run() {
		GLFWwindow* window;
		glfwSetErrorCallback(error_callback);

		if (!glfwInit())
			exit(EXIT_FAILURE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		window = glfwCreateWindow(640, 480, "ehjE Simple example", NULL, NULL);
		ehjSetGLFWicon(window);
		if (!window) {
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetKeyCallback(window, key_callback);
		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		ehj_gl_err_callback();
		glfwSwapInterval(1);

		if (0) { // print max sizes
			ivec3 maxWorkGroupSize;
			ivec3 maxWorkGroupCount;
			int maxWorkGroupInv;
			for (int i=0;i<3;++i) {
				glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,i,&(maxWorkGroupSize[i]));
				glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,i,&(maxWorkGroupCount[i]));
				glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,&maxWorkGroupInv);
			}
			std::cout << "maxWorkGroupSize " << glm::to_string(maxWorkGroupSize) 
				<< " max size of work group (SIMD thread group)" << std::endl;
			std::cout << "maxWorkGroupCount " << glm::to_string(maxWorkGroupCount) 
				<< " max size of grid" << std::endl;
			std::cout << "maxWorkGroupInv " << maxWorkGroupInv << std::endl;
		}

		//example1_testPi();
		//example2_testImg(window);
		example3_ssbo();

		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_SUCCESS);
		return 0;
	}
	void cleanup() {

	}
};
