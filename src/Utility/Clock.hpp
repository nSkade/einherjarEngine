#include <chrono>

namespace ehj {

class Clock {
public:
	Clock () {
		m_startTime = std::chrono::steady_clock::now();
	}
	float update() {
		auto elapsed = std::chrono::steady_clock::now() - m_startTime;
		m_time = float(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count())*0.000001;
		m_startTime = std::chrono::steady_clock::now();
		return m_time;
	};
	float getDeltaTime() { return m_time; };

private:
	std::chrono::steady_clock::time_point m_startTime;
	float m_time;
};

}//ehj
