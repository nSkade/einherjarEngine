#pragma once
#include "glad/glad.h"

class GPUTimer {
	
public:
	void startQuery();
	void printQuery();
private:
	GLuint m_queryID;
	bool m_query_read = true;
};

