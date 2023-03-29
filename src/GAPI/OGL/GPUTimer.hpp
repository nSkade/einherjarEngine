#pragma once

#include <glad/glad.h>

class GPUTimer {
	
public:
	void start();
	void end();
	void print();
	double getMS();
private:
	GLuint m_queryID;
	bool m_query_read = true;
	double m_ms;
};