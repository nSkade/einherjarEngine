#include "GPUTimer.hpp"
#include <iostream>

void GPUTimer::startQuery() {
	if (m_query_read) {
		glGenQueries(1, &m_queryID);
		glBeginQuery(GL_TIME_ELAPSED, m_queryID);
	}
}

void GPUTimer::printQuery() {
	if (m_query_read)
		glEndQuery(GL_TIME_ELAPSED);
	GLint queryState;
	glGetQueryObjectiv(m_queryID, GL_QUERY_RESULT_AVAILABLE, &queryState);
	if (queryState) {
		GLuint64 time;
		glGetQueryObjectui64v(m_queryID, GL_QUERY_RESULT, &time);
		double ms = ((double) time)/1000000.0;
		std::cout << "fps: " << 1.0/(ms/1000.0) << " time for render took: " << ms << " id: " << m_queryID << "\n";
		glDeleteQueries(1, &m_queryID);
		m_queryID = 0;
		m_query_read = true;
	} else {
		m_query_read = false;
	}
}
