#include "utils/Timer.hpp"
#include <iostream>

namespace clustering {

    Timer::Timer() : m_isRunning(false) {}

    void Timer::start() {
        m_start = std::chrono::high_resolution_clock::now();
        m_isRunning = true;
    }

    void Timer::stop() {
        if (m_isRunning) {
            m_end = std::chrono::high_resolution_clock::now();
            m_isRunning = false;
        }
    }

    void Timer::reset() {
        m_isRunning = false;
    }

    double Timer::getElapsedMs() const {
        if (m_isRunning) {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::milli>(now - m_start).count();
        } else {
            return std::chrono::duration<double, std::milli>(m_end - m_start).count();
        }
    }

    double Timer::getElapsedSeconds() const {
        return getElapsedMs() / 1000.0;
    }

    // ----- Scoped -----

    Timer::Scoped::Scoped(const std::string& name)
        : m_timer(std::make_unique<Timer>()), m_name(name) {
        m_timer->start();
    }

    Timer::Scoped::~Scoped() {
        m_timer->stop();
        if (!m_name.empty()) {
            std::cout << "[Timer] " << m_name << ": "
                      << m_timer->getElapsedMs() << " ms\n";
        }
    }

    double Timer::Scoped::elapsedMs() const {
        return m_timer->getElapsedMs();
    }

} // namespace clustering