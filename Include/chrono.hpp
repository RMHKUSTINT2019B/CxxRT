#pragma once

#include "ch.h"
#include <chrono>

namespace chstd {
    class steady_clock {
    public:
        using rep = long long;
        using period = std::micro;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<steady_clock>;

        static constexpr bool is_steady() noexcept { return true; }

        static time_point now() noexcept {
            return steady_clock::time_point(std::chrono::microseconds(TIME_I2US(chVTGetSystemTimeX())));
        }
    };

    using high_resolution_clock = steady_clock;
}
