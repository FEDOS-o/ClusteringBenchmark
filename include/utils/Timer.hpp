#pragma once
#include <chrono>
#include <string>

namespace clustering {

    class Timer {
    public:
        Timer();
        void start();
        void stop();
        void reset();
        double getElapsedMs() const;
        double getElapsedSeconds() const;

        class Scoped {
        public:
            Scoped(const std::string& name = "");
            ~Scoped();
            double elapsedMs() const;
        private:
            Timer m_timer;
            std::string m_name;
        };

    private:
        std::chrono::high_resolution_clock::time_point m_start;
        std::chrono::high_resolution_clock::time_point m_end;
        bool m_isRunning = false;
    };

} // namespace clustering