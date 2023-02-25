#include "../src/suOGL.hpp"
#include "../src/Input/GLFW/GLFWKeyboard.hpp"
#include "../src/Input/GLFW/GLFWMouse.hpp"
#include "../src/Input/GLFW/GLFWCallbackTest.hpp"


using namespace ehj;

#define FULLSCREEN false

class EnvirScene : IScene {
public:

	~EnvirScene() {
		//__debugbreak();
	}
	void setup() {
		if (!glfwInit())
			exit(EXIT_FAILURE);
	
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

		m_windowRes.x = 640;
		m_windowRes.y = 480;
	
	#if FULLSCREEN
		m_windowRes.x = 1920;
		m_windowRes.y = 1080;
	#endif

		m_pWindow = glfwCreateWindow(m_windowRes.x,m_windowRes.y, "EnvirScene", NULL, NULL);
	
		if (!m_pWindow)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
	#if FULLSCREEN
		glfwSetWindowPos(m_pWindow, 0,0);
	#endif

		// make sure to keep instances alive
		m_kb = ehj::GLFWKeyboard::instance();
		m_mouse = ehj::GLFWMouse::instance();

		//glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//TODO set with imgui, exit with escape
		glfwSetCursorPosCallback(m_pWindow, m_mouse->mouse_callback);
		glfwSetMouseButtonCallback(m_pWindow, m_mouse->mouse_button_callback);
		glfwSetKeyCallback(m_pWindow, m_kb->key_callback);
	}

	int run() {
		GLuint program;
		GLint pvm_location;

		//glfwSetErrorCallback(error_callback); //TODO

		//TODO
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		//ehj::Camera cam;
		//ehj::CameraController camContr(mouse,kb,&cam);
		
		//GLFWCallbackTest cbt;

		glfwMakeContextCurrent(m_pWindow);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	
		GLProgram mainGLProgram;
		
		// load model
		ehj::Mesh mesh; mesh.loadOBJ("models/monkey.obj");
		//mesh.toTriangles();
		OGLMesh oglMesh(mesh, GL_DYNAMIC_DRAW);
		oglMesh.bind(0);

		ehj_gl_err();
		glBindVertexArray(oglMesh.getVAO());
		ehj_gl_err();
		
		// tesselation maximum supported vertices
		//GLint MaxPatchVertices = 0;
		//glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		//std::cout << "Max supported patch vertices "<< MaxPatchVertices << "\n";
		//glPatchParameteri(GL_PATCH_VERTICES, 4);
		ehj_gl_err();

		//mainGLProgram.addSourceFromFile("shaders/basic_v.vert", GL_VERTEX_SHADER);
		//mainGLProgram.addSourceFromFile("shaders/basic_f.frag",GL_FRAGMENT_SHADER);
		//mainGLProgram.addSourceFromFile("shaders/tellu.frag",GL_FRAGMENT_SHADER);
		
		mainGLProgram.loadProgramFromFolder("shaders");
		mainGLProgram.addSourceFromFile("shaders/basic_f.frag");
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
		pvm_location = glGetUniformLocation(program, "pvm");
		GLint utime_location = glGetUniformLocation(program, "u_time");
		GLint ures_location = glGetUniformLocation(program, "u_resolution");
		GLint uview_location = glGetUniformLocation(program, "u_view");

		GLint tessQ_location = glGetUniformLocation(program, "u_tessQ");
		std::cout << "\n tessQ: " << tessQ_location << "\n";
	
		std::cout << program << " " <<pvm_location << " " << utime_location << " " << ures_location;
	
		std::cout << "check2\n";
		ehj_gl_err();
		GPUTimer fragSTimer;
	
		//glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CW);

		//proj
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)m_windowRes.x/(float)m_windowRes.y,0.0f,1000.0f);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int guiTess = 1;

		while (!glfwWindowShouldClose(m_pWindow))
		{
			float ratio;
			int width, height;
			
			//camera
			//static glm::vec3 camPos = glm::vec3(0.0f,0.0f,3.0f);
			//static glm::vec3 camTar = glm::vec3(0.0f,0.0f,0.0f);
			//static glm::vec3 camDir = glm::normalize(camPos-camTar);

			//glm::vec3 direction = glm::vec3(0.0f,0.0f,-1.0f);
		//	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		//	direction.y = sin(glm::radians(pitch));
		//	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			//camDir = glm::normalize(direction);

			//m_cam.setDir(camDir);
			m_cam.update(0.05f);
		
			//static glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);
			//static glm::vec3 camRight = glm::normalize(glm::cross(camUp,camDir));
			//camUp = glm::cross(camDir,camRight);
	
			//const float cameraSpeed = 0.05f; // adjust accordingly
			//if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
			//	camPos += cameraSpeed * (-camDir);
			//if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
			//	camPos -= cameraSpeed * (-camDir);
			//if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
			//	camPos -= glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;
			//if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
			//	camPos += glm::normalize(glm::cross(-camDir, camUp)) * cameraSpeed;

			//glm::mat4 view = glm::lookAt(camPos,camPos-camDir,camUp);
		
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			m_windowRes = glm::ivec2(width,height);

			ratio = (float) width / (float) height;
		
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindVertexArray(oglMesh.getVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglMesh.getEBO());

		//	glm::mat4 m = glm::mat4(1.0f); // identity
		//	m = glm::rotate(m,(float) glfwGetTime(), glm::vec3(0.f,1.f,0.f));
		//	m = glm::translate(m,glm::vec3(0.0f,0.0f,-1.0f));
		//	glm::mat4 p = glm::ortho(-1.f,1.f,-1.f,1.f);
		//	//glm::mat4 p = glm::perspectiveFov(45,10,10,0,1000);
		//	glm::mat4 pvm = p*m;
			glm::mat4 proj = glm::perspective(glm::radians(90.0f), (float)m_windowRes.x/(float)m_windowRes.y,0.0f,1000.0f);
			glm::mat4 view = m_cam.getView();
			proj = proj*view;
			glm::mat4 pvm = proj; //TODO rename to pvm
		
			glUseProgram(program);
			glUniformMatrix4fv(pvm_location, 1, GL_FALSE, &pvm[0][0]);
		
			//glUniform1f(utime_location, utime);
			glUniform2f(ures_location, width, height);
			glUniform1i(tessQ_location, (GLint) guiTess);
		
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

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();
		}
		return 0;
	}
	void cleanup() {
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();

		ImGui::DestroyContext();

		//delete m_cam;
	}
private:
	GLFWwindow* m_pWindow;
	glm::ivec2 m_windowRes;

	// to hold references to instances
	std::shared_ptr<ehj::GLFWKeyboard> m_kb;
	std::shared_ptr<ehj::GLFWMouse> m_mouse;

	FreeFlyCamera m_cam; // TODO when not pointer adress of variable changes -> nullptr except
};
