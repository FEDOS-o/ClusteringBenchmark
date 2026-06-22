#pragma once
#include <chrono>
#include <string>
#include <memory>

namespace clustering {

    class Timer {
    public:
        Timer();
        ~Timer() = default;

        void start();
        void stop();
        void reset();
        double getElapsedMs() const;
        double getElapsedSeconds() const;

        class Scoped {
        public:
            explicit Scoped(const std::string& name = "");
            ~Scoped();

            double elapsedMs() const;

        private:
            std::unique_ptr<Timer> m_timer;  // Используем unique_ptr
            std::string m_name;
        };

    private:
        std::chrono::high_resolution_clock::time_point m_start;
        std::chrono::high_resolution_clock::time_point m_end;
        bool m_isRunning = false;
    };

} // namespace clustering