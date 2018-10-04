#pragma once

#include <chrono>
#include <utility>
#include "ch.h"

namespace chstd {
    namespace this_thread {
        using namespace std::chrono;

        inline void yield() noexcept { chThdYield(); }

        template<class Rep, class Period>
        inline void sleep_for(const duration<Rep, Period> &dr) noexcept {
            chThdSleepMicroseconds(duration_cast<microseconds>(dr).count());
        }

        template<class Clock, class Dura>
        inline void sleep_until(const time_point<Clock, Dura> &time) { sleep_for(time - Clock::now()); }
    }

    class thread {
        template<class Callable>
        using _rc = std::pair<Callable &&, bool>;

        template<class Callable>
        static void _fn(void *u) {
            auto &_r = *reinterpret_cast<_rc<Callable> *>(u);
            Callable _f = std::move(_r.first);
            _r.second = false;
            _f();
        }

    public:
        using native_handle_type = thread_t*;

        using id = native_handle_type;

        thread() = default;

        template<class Fn>
        explicit thread(Fn fn) noexcept {
            auto _r = _rc<Fn>(std::move(fn), true);
            _thrd = chThdCreateFromHeap(nullptr, 256, "", NORMALPRIO, &_fn<Fn>, &_r);
            while (_r.second) this_thread::yield();
        }

        thread(thread &&) = default;

        thread &operator=(thread &&) = default;

        thread(const thread &) = delete;

        thread &operator=(const thread &) = delete;

        ~thread() noexcept {
            if (_thrd) {
                if (_thrd->state != CH_STATE_FINAL) {
                    chSysHalt("TR");
                }
                chThdRelease(_thrd);
            }
        }

        native_handle_type native_handle() noexcept { return _thrd; }

        id get_id() const noexcept { return _thrd; }

        bool joinable() const noexcept { return _thrd; }

        void join() noexcept { chThdWait(_thrd); }

        void detach() noexcept {
            chThdRelease(_thrd);
            _thrd = nullptr;
        }

        void swap(thread& other) noexcept { std::swap(_thrd, other._thrd); }

        static constexpr unsigned int hardware_concurrency() noexcept { return 1; }

    private:
        thread_t *_thrd = nullptr;
    };

    namespace this_thread {
        inline thread::id get_id() noexcept { return chThdGetSelfX(); }
    }
}
