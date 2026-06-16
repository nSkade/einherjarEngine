#include "GLProgram.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <string>


namespace fs = std::filesystem;

GLProgram::GLProgram() {
}

GLProgram::~GLProgram() {
	if (m_programID != -1)
		glDeleteProgram(m_programID);
}

void GLProgram::createProgram() {
	clearUniformLocations();
	if (m_programID != -1)
		glDeleteProgram(m_programID);
	m_programID = glCreateProgram();
	auto itr = m_shaders.toEnum.begin();
	while (itr != m_shaders.toEnum.end()) {
		glAttachShader(m_programID, itr->first);
		itr++;
	}
	glLinkProgram(m_programID);
	for (auto id : m_shaders.toEnum) {
		glDeleteShader(id.first);
	}
	m_shaders.clear();
}

void GLProgram::bind() {
	glUseProgram(m_programID);
}

bool GLProgram::addSourceFromString(std::string shaderSource, GLenum shaderType, const std::string& filePath) {
	auto itr = m_shaders.toID.find(shaderType);
	if (itr != m_shaders.toID.end()) { // shader does already exist
		std::cout << "Note: loaded shader overwritten\n";
		GLuint oldID = itr->second;
		glDeleteShader(oldID);
		m_shaders.remove(oldID);
	}
	GLuint shaderID = glCreateShader(shaderType);
	if (shaderID == 0)
		throw std::runtime_error("GLProgram::addSourceFromString : glCreateShader error occured");
	
	const char* shaderSourceC = shaderSource.c_str();
	glShaderSource(shaderID, 1, &shaderSourceC, NULL);
	glCompileShader(shaderID);
	GLint success = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<GLchar> log(logSize);
		glGetShaderInfoLog(shaderID, logSize, NULL, &log[0]);
		std::string logSTR = log.data();
		if (filePath != "")
			std::cerr << "Shader error: " << filePath << std::endl;
		else
			std::cerr << "Shader error: " << shaderSource << std::endl;

		std::cerr << logSTR << std::endl;

		glDeleteShader(shaderID);
		return false;
	}

	m_shaders.push_back(shaderID,shaderType);
	return true;
}

std::string GLProgram::loadFileContents(std::string path) {
	std::stringstream buffer;
	std::filesystem::path p(path);
	if (!std::filesystem::exists(p))
		throw std::runtime_error("file: "+p.string()+" does not exist");

	std::ifstream t(p.string());
	buffer << t.rdbuf();
	std::string contents = buffer.str();
	return contents;
}

bool GLProgram::addSourceFromFile(std::string shaderPath, GLenum shaderType) {
	std::string shaderString = loadFileContents(shaderPath);
	return addSourceFromString(shaderString,shaderType,shaderPath);
}

////TODO not implemented
//void GLProgram::loadProgramFromFilename(std::string folderPath, std::string fileName) {
//	std::cerr << "loadProgramFromFilename not implemented yet\n";
//	return;
//	for (const auto & entry : fs::directory_iterator(folderPath)) {
//		std::cout << entry.path() << std::endl;
//	
//	}
//}

GLenum GLProgram::detectShaderType(std::string fileName) {
	GLenum shaderType = GL_INVALID_ENUM;
	
	std::string name = fileName;

	if (name.find(".vert") != name.npos |
	    name.find(".vs") != name.npos |
	    name.find("_v.") != name.npos)
		shaderType = GL_VERTEX_SHADER;
	
	if (name.find(".frag") != name.npos |
	    name.find(".fs") != name.npos |
	    name.find("_f.") != name.npos)
		shaderType = GL_FRAGMENT_SHADER;

	if (name.find("tcs") != name.npos |
		name.find(".tesc") != name.npos)
		shaderType = GL_TESS_CONTROL_SHADER;

	if (name.find("tes") != name.npos |
		name.find(".tese") != name.npos)
		shaderType = GL_TESS_EVALUATION_SHADER;
	
	if (name.find(".comp") != name.npos)
		shaderType = GL_COMPUTE_SHADER;
	
	if (name.find(".geom") != name.npos)
		shaderType = GL_GEOMETRY_SHADER;
	return shaderType;
}

void GLProgram::loadProgramFromFolder(std::string folderPath) {
	for (const auto & entry : fs::directory_iterator(folderPath)) {
		if (entry.is_directory())
			continue;
		
		GLenum shaderType = detectShaderType(entry.path().filename().string());

	#ifdef EHJ_DBG
		std::cout << entry.path() << std::endl;
		switch (shaderType)
		{
		case GL_FRAGMENT_SHADER:
			std::cout << "GL_FRAGMENT_SHADER\n";
			break;
		case GL_VERTEX_SHADER:
			std::cout << "GL_VERTEX_SHADER\n";
			break;
		case GL_TESS_CONTROL_SHADER:
			std::cout << "GL_TESS_CONTROL_SHADER\n";
			break;
		case GL_TESS_EVALUATION_SHADER:
			std::cout << "GL_TESS_EVALUATION_SHADER\n";
			break;
		case GL_GEOMETRY_SHADER:
			std::cout << "GL_GEOMETRY_SHADER\n";
			break;
		default:
			break;
		}
	#endif

		if (shaderType != GL_INVALID_ENUM)
			addSourceFromFile(entry.path().string(),shaderType);
	}
}

bool GLProgram::addSourceFromFile(std::string shaderPath) {
	std::string shaderString = loadFileContents(shaderPath);

	std::filesystem::path sp(shaderPath);
	GLenum shaderType = detectShaderType(sp.filename().string());

	return addSourceFromString(shaderString,shaderType,shaderPath);
}

GLint GLProgram::getUnfLoc(std::string name) {
	GLint loc;
	if (m_uniformLocations.find(name) != m_uniformLocations.end())
		loc = m_uniformLocations[name];
	else { // uniform location does not exist
		loc = glGetUniformLocation(m_programID, name.c_str());
		m_uniformLocations[name] = loc;
	}
	return loc;
}

GLint GLProgram::getAttribLoc(std::string name) {
	GLint loc;
	if (m_attribLocations.find(name) != m_attribLocations.end())
		loc = m_attribLocations[name];
	else { // attrib location does not exist
		loc = glGetAttribLocation(m_programID, name.c_str());
		m_attribLocations[name] = loc;
	}
	return loc;
}

void GLProgram::clearUniformLocations() {
	m_uniformLocations.clear();
}

bool GLProgram::addSourceFromFileRecursive(std::string shaderPath, GLenum shaderType) {
	std::string shaderString = loadFileContents(shaderPath);
	std::string outString;

	int fileID=0;
	std::vector<std::string> fileNames;
	resolveInclude(shaderPath,0,&fileID,&outString,&fileNames);

	//std::cout << "GLProgram::addSourceFromFileRecursive resolved start" << std::endl;
	//std::cout << outString << std::endl;
	//std::cout << "GLProgram::addSourceFromFileRecursive resolved end" << std::endl;

	if (!addSourceFromString(outString,shaderType,shaderPath)) {
		int i=0;
		for (auto& f : fileNames) {
			std::cout << "fileID: " << i << " - " << f << "\n";
			i++;
		}
		return false;
	}

	return true;
}

bool GLProgram::addSourceFromFileRecursive(std::string shaderPath) {
	std::filesystem::path sp(shaderPath);
	GLenum shaderType = detectShaderType(sp.filename().string());
	return addSourceFromFileRecursive(shaderPath, shaderType);
}

//TODO add check to avoid include loop or double includes which could casue conflicts
void GLProgram::resolveInclude(const std::string shaderPath, int curDepth, int* fileID, std::string* outSource, std::vector<std::string>* fileNames) {
	if (curDepth >= m_maxIncludeDepth)
		std::cerr << "GLProgram::resolveInclude error: max include depth reached!" << std::endl;

	std::string shaderSource = loadFileContents(shaderPath);

	int currLineNum=0;
	int sourceProgIdx=0;
	int currFileID=*fileID;
	std::string fileName=std::filesystem::path(shaderPath).filename().string();
	fileNames->push_back(fileName);
	(*fileID)++;

	for (int i=0;i<shaderSource.size()-1;++i) {
		if (i==0 && shaderSource.compare(0,8,"#include") == 0)
			std::cerr << "GLProgram::resolveInclude warning: " + fileName
				+ " cannot put #include as first line yet" << std::endl;
		if (shaderSource.at(i) == '\n') {
			currLineNum++;
			if (shaderSource.at(i+1) == '#') {
				if (shaderSource.compare((i+2),8,"include ") == 0) { // +2 to skip \n#

					outSource->insert(outSource->end(),shaderSource.begin()+sourceProgIdx,shaderSource.begin()+i);
					{ // prepend #line directive for debugging: #line [line] [source-string-number]
						std::string directive = "\n#line 1 " + std::to_string(*fileID) + "\n"; // glsl spec sadly only accepts int here not file name
						outSource->insert(outSource->end(),directive.begin(),directive.end());
					}

					i+=10; // skip "\n#include "

					std::string newPath = "";
					{// skip include marker
						if (shaderSource.at(i)!='\"')
							continue;
						i++;
						char c = shaderSource.at(i);
						while (c!='\"') {
							newPath += c;
							c = shaderSource.at(++i);
						}
						i++; // skip last "
					}
					std::filesystem::path sp(shaderPath);
					std::string includeCode = "";
					
					//check relative path
					std::string relativeGlobal = sp.parent_path().string()+"/"+newPath;
					if (std::filesystem::exists(relativeGlobal)) {
						includeCode = loadFileContents(relativeGlobal);
						resolveInclude(relativeGlobal,curDepth+1,fileID, outSource,fileNames);
					} else if (std::filesystem::exists(newPath)) {
						// check global path
						includeCode = loadFileContents(newPath);
						resolveInclude(newPath,curDepth+1,fileID, outSource,fileNames);
					} else {
						std::cerr << "GLProgram::resolveInclude warning: " + newPath + " could not be resolved" << std::endl;
						continue;
					}
					
					{ // end line marker
						std::string reset = "\n#line " + std::to_string(currLineNum-1) + " " + std::to_string(currFileID) + "\n";
						outSource->insert(outSource->end(),reset.begin(),reset.end());
					}
					sourceProgIdx=i;
					i--; // avoid skip if 2 includes are only seperated by \n
				}// if include
			}// if #
		}// \n
	}
	outSource->insert(outSource->end(),shaderSource.begin()+sourceProgIdx,shaderSource.end());
	//if (currFileID==0) {
	//	std::system("clear");
	//	std::cout << *outSource << "\n";
	//}
};
