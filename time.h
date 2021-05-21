#pragma once

#include <iostream>
#include "eassert.h"

// тики ведутся с 1-го января 1970 г. 00:00:00 Всемирного времени

#if defined(_WIN32)

#include <windows.h>

// вернет частоту обновления устройства
uint64_t get_performance_frequency() {
    LARGE_INTEGER perf;

    EASSERT(QueryPerformanceFrequency(&perf), "call to QueryPerformanceFrequency fails");

    return perf.QuadPart;
}

// вернет текущий тик
uint64_t get_ticks() {
    LARGE_INTEGER ticks;

    EASSERT(QueryPerformanceCounter(&ticks), "call to QueryPerformanceCounter fails");

    return ticks.QuadPart;
}

#elif defined(__linux__) || defined (__APPLE__)

#include <sys/time.h>

// вернет частоту обновления устройства
uint64_t get_performance_frequency() {
    return 1'000'000; // колво микросекунд в секунде
}

// вернет текущий тик
uint64_t get_ticks() {
    timeval ticks;

    // вернет -1 в случае ошибки
    EASSERT(gettimeofday(&ticks, NULL) == 0, "call to gettimeofday fails");
    return ticks.tv_usec;
}

#else

// not supported operating system

#endif

const double performance_frequency = get_performance_frequency();

class Timer {

    uint64_t start_tick;

public:

    Timer() {
        reset();
    }

    // вернет время после reset|default constructor в секундах
    double time() const {

        // разница тиков делить на частоту
        return (get_ticks() - start_tick) / performance_frequency;
    }

    // обновит указатель времени
    void reset() {
        start_tick = get_ticks();
    }

    // вернет тик начала отсчета
    uint64_t get_tick() const {
        return start_tick;
    }
};
std::ostream& operator << (std::ostream& output, const Timer& time) {
    return output << time.time() << "s";
}
