#include <chrono>

namespace ehj {

/**
 * @brief Measure a time interval on CPU
*/
class Timer {
public:
	Timer() {
		m_t1 = std::chrono::high_resolution_clock::now();
	}

	void startTimer() {
		m_t1 = std::chrono::high_resolution_clock::now();
	}

	double endTimer() {
		auto elapsed = std::chrono::steady_clock::now() - m_t1;
		m_interval = 0.001*std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		//auto us_double = std::chrono::duration_cast<std::chrono::microseconds>(t2 - m_t1);
		//m_interval = 0.001*us_double.count();
		return m_interval;
	}

	/**
	 * @brief Get last Measurement in milliseconds
	*/
	double getInterval() { return m_interval; };

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_t1;
	double m_interval = 16.0;
};

}//ehj
