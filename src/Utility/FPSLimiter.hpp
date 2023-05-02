
#include <thread>
#include <chrono>

namespace ehj {

/**
 * @brief CPU based wait until target Time is reached
*/
class FPSLimiter {
public:
	FPSLimiter(uint32_t fpsLimit = 60) {
		m_fpsLimit = fpsLimit; 
		calcInterval();
		m_tp = std::chrono::high_resolution_clock::now();
	};
	void setLimit(uint32_t fpsLimit) { m_fpsLimit = fpsLimit; calcInterval(); };
	
	void wait() {
		m_tp += std::chrono::microseconds(m_interval);
		std::this_thread::sleep_until(m_tp);
	}

private:
	void calcInterval() {
		m_interval = (double) 1.0/m_fpsLimit*1000.0*1000.0;
	}
	std::chrono::high_resolution_clock::time_point m_tp;
	uint32_t m_fpsLimit = 60;
	uint32_t m_interval;
};

}//ehj
