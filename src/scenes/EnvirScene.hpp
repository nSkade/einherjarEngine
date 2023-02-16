#include "../suOGL.hpp"
#include "../Input.hpp"

using namespace ehj;

class EnvirScene : IScene {
public:
	void setup() {

	}
	int run() {
		GLFWwindow* window;
		GLuint program;
		GLint mvp_location;

		//glfwSetErrorCallback(error_callback);

		if (!glfwInit())
			exit(EXIT_FAILURE);
	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		uint32_t windowW, windowH;
		windowW = 640;
		windowH = 480;
	
	#if FULLSCREEN
		windowW = 1920;
		windowH = 1080;
	#endif
		window = glfwCreateWindow(windowW,windowH, "Simple example", NULL, NULL);
	
		if (!window)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	#if FULLSCREEN
		glfwSetWindowPos(window, 0,0);
	#endif

		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
		//TODO mouse callback not in class possible
		//glfwSetCursorPosCallback(window, mouse_callback);
		//glfwSetMouseButtonCallback(window, mouse_button_callback);
	
		Mouse* mouse = Mouse::instance();
		Keyboard* kb = Keyboard::instance();
		ehj::Camera cam(mouse, kb);
		
		glfwSetCursorPosCallback(window, mouse->mouse_callback);
		glfwSetMouseButtonCallback(window, mouse->mouse_button_callback);
		glfwSetKeyCallback(window, kb->key_callback);

		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	
		GLProgram mainGLProgram;
		
		// load model
		ehj::Mesh mesh; mesh.loadOBJ("models/ssn4.obj");
		mesh.toTriangles();
		OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
		oglMesh.bind(0);

		ehj_gl_err();
		glBindVertexArray(oglMesh.getVAO());
		ehj_gl_err();
		
		// tesselation maximum supported vertices
		//GLint MaxPatchVertices = 0;
		//glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		//std::cout << "Max supported patch vertices "<< MaxPatchVertices << "\n";
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		ehj_gl_err();

		mainGLProgram.addSourceFromFile("shaders/basic_v.vert", GL_VERTEX_SHADER);
		mainGLProgram.addSourceFromFile("shaders/basic_f.frag",GL_FRAGMENT_SHADER);
		//mainGLProgram.addSourceFromFile("shaders/tellu.frag",GL_FRAGMENT_SHADER);
		
		//mainGLProgram.loadProgramFromFolder("shaders");
		//mainGLProgram.addSourceFromFile("shaders/basic_v.vert", GL_FRAGMENT_SHADER);
		//mainGLProgram.loadProgramFromFolder("shaders/tessQ/");
		//mainGLProgram.addSourceFromFile("shaders/tessQ/basic_tesQ.glsl",GL_TESS_EVALUATION_SHADER);

		ehj_gl_err();
		mainGLProgram.createProgram();
		ehj_gl_err();
		program = mainGLProgram.getProgramID();
		glBindAttribLocation(program,oglMesh.getAttribPos(),"vPos");
		if (oglMesh.getAttribNrm()!=-1)
			glBindAttribLocation(program,oglMesh.getAttribNrm(),"vNrm");
		ehj_gl_err();
		glUseProgram(program);

		std::cout << "check1\n";
		ehj_gl_err();
		mvp_location = glGetUniformLocation(program, "MVP");
		GLint utime_location = glGetUniformLocation(program, "u_time");
		GLint ures_location = glGetUniformLocation(program, "u_resolution");
		GLint uview_location = glGetUniformLocation(program, "u_view");

		GLint tessQ_location = glGetUniformLocation(program, "u_tessQ");
		std::cout << "\n tessQ: " << tessQ_location << "\n";
	
		std::cout << program << " " <<mvp_location << " " << utime_location << " " << ures_location;
	
		std::cout << "check2\n";
		ehj_gl_err();
		GPUTimer fragSTimer;
	
		//glLineWidth(1.0f);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CW);

		//proj
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)windowW/(float)windowH,0.0f,1000.0f);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int guiTess = 1;
	
		while (!glfwWindowShouldClose(window))
		{
			float ratio;
			int width, height;
			//mat4x4 m, p, mvp;

			//camera
			static glm::vec3 camPos = glm::vec3(0.0f,0.0f,3.0f);
			static glm::vec3 camTar = glm::vec3(0.0f,0.0f,0.0f);
			static glm::vec3 camDir = glm::normalize(camPos-camTar);

			glm::vec3 direction = glm::vec3(0.0f,0.0f,-1.0f);
		//	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		//	direction.y = sin(glm::radians(pitch));
		//	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			camDir = glm::normalize(direction);
		
			static glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);
			static glm::vec3 camRight = glm::normalize(glm::cross(camUp,camDir));
			camUp = glm::cross(camDir,camRight);
	
			const float cameraSpeed = 0.05f; // adjust accordingly
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				camPos += cameraSpeed * (-camDir);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				camPos -= cameraSpeed * (-camDir);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				camPos -= glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				camPos += glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;
		
			glm::mat4 view = glm::lookAt(camPos,camPos-camDir,camUp);
		
			glfwGetFramebufferSize(window, &width, &height);
		
			ratio = (float) width / (float) height;
		
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindVertexArray(oglMesh.getVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());

			glm::mat4 m = glm::mat4(1.0f); // identity
			//m = glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
			//m = glm::translate(m,glm::vec3(0.0f,0.0f,0.0f));
			glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
			//glm::mat4 p = glm::perspectiveFov(45,10,10,0,1000);
			glm::mat4 mvp = p*m;
			//mvp = proj;
		
			glUseProgram(program);
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
		
			//glUniform1f(utime_location, utime);
			glUniform2f(ures_location, width, height);
			glUniform1i(tessQ_location, (GLint) guiTess);

			//const float radius = 10.0f;
			//float camX = sin(glfwGetTime()) * radius;
			//float camZ = cos(glfwGetTime()) * radius;
			//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
			view = glm::mat4(1.0f);
			glUniformMatrix4fv(uview_location,1,GL_FALSE,glm::value_ptr(view));
		
			ehj_gl_err();
		
			fragSTimer.start();
			//glDrawArrays(GL_TRIANGLES, 0, 6);
			//glDrawElements(GL_TRIANGLES,EBO.size(),GL_UNSIGNED_INT,0);
			//glDrawArrays(GL_PATCHES,0,6);
			glDrawElements(GL_TRIANGLES,oglMesh.getEBOsize(),GL_UNSIGNED_INT,0);
			//glDrawArrays(GL_QUADS,0,EBO.size());
			//glDrawElements(GL_PATCHES, EBO.size(),GL_UNSIGNED_INT,0);
			fragSTimer.end();
			//fragSTimer.print();

			ehj_gl_err();

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		glfwDestroyWindow(window);
	
		glfwTerminate();
		return 0;
	}
	void cleanup() {

	}
};

