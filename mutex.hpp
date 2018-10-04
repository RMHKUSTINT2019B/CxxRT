#pragma once

#include "ch.h"
#include <mutex>

namespace chstd {
    namespace _details {
        class __unfair_user_space_mtx_base {
        public:
            using native_handle_type = semaphore_t &;

            constexpr __unfair_user_space_mtx_base() noexcept = default;

            __unfair_user_space_mtx_base(const __unfair_user_space_mtx_base &) = delete;

            __unfair_user_space_mtx_base &operator=(const __unfair_user_space_mtx_base &) = delete;

            __unfair_user_space_mtx_base(__unfair_user_space_mtx_base &&) = delete;

            __unfair_user_space_mtx_base &operator=(__unfair_user_space_mtx_base &&) = delete;

            ~__unfair_user_space_mtx_base() noexcept = default;

            native_handle_type native_handle() noexcept { return _mtx; }

        protected:
            void lock() noexcept { chSemWait(&_mtx); }

            bool try_lock() noexcept { return try_lock_for_us(0); }

            template<class Rep, class Period>
            bool try_lock_for(const std::chrono::duration<Rep, Period> &d) noexcept {
                return try_lock_for_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
            }

            template<class Clock, class Duration>
            bool try_lock_until(const std::chrono::time_point<Clock, Duration> &t) noexcept {
                return try_lock_for(t - Clock::now());
            }

            void unlock() noexcept { chSemSignal(&_mtx); }

        private:
            bool try_lock_for_us(const long long int us) noexcept {
                return chSemWaitTimeout(&_mtx, TIME_US2I(us)) != MSG_TIMEOUT;
            }

            SEMAPHORE_DECL(_mtx, 1);
        };

        using __mtx_base = __unfair_user_space_mtx_base;
    }

    class mutex : public _details::__mtx_base {
    public:
        constexpr mutex() noexcept = default;

        using _details::__mtx_base::lock;

        using _details::__mtx_base::try_lock;

        using _details::__mtx_base::unlock;
    };

    class timed_mutex : public _details::__mtx_base {
    public:
        constexpr timed_mutex() noexcept = default;

        using _details::__mtx_base::lock;

        using _details::__mtx_base::try_lock;

        using _details::__mtx_base::try_lock_for;

        using _details::__mtx_base::try_lock_until;

        using _details::__mtx_base::unlock;
    };

    template<typename _Mutex>
    class unique_lock {
    public:
        using mutex_type = _Mutex;

        unique_lock() noexcept: _device(0), _owns(false) {}

        explicit unique_lock(mutex_type &m) : _device(std::addressof(m)), _owns(false) {
            lock();
            _owns = true;
        }

        unique_lock(mutex_type &m, std::defer_lock_t) noexcept : _device(std::addressof(m)), _owns(false) {}

        unique_lock(mutex_type &m, std::try_to_lock_t) : _device(std::addressof(m)), _owns(_device->try_lock()) {}

        unique_lock(mutex_type &m, std::adopt_lock_t) noexcept: _device(std::addressof(m)), _owns(true) {}

        template<typename _Clock, typename _Duration>
        unique_lock(mutex_type &m, const std::chrono::time_point<_Clock, _Duration> &atime)
                : _device(std::addressof(m)), _owns(_device->try_lock_until(atime)) {}

        template<typename _Rep, typename _Period>
        unique_lock(mutex_type &m, const std::chrono::duration<_Rep, _Period> &rtime)
                : _device(std::addressof(m)), _owns(_device->try_lock_for(rtime)) {}

        ~unique_lock() {
            if (_owns)
                unlock();
        }

        unique_lock(const unique_lock &) = delete;

        unique_lock &operator=(const unique_lock &) = delete;

        unique_lock(unique_lock &&u) noexcept : _device(u._device), _owns(u._owns) {
            u._device = 0;
            u._owns = false;
        }

        unique_lock &operator=(unique_lock &&u) noexcept {
            if (_owns)
                unlock();

            unique_lock(std::move(u)).swap(*this);

            u._device = 0;
            u._owns = false;

            return *this;
        }

        void lock() {
            if (!_device || _owns)
                chSysHalt("");
            else {
                _device->lock();
                _owns = true;
            }
        }

        bool try_lock() {
            if (!_device || _owns)
                chSysHalt("");
            else {
                _owns = _device->try_lock();
                return _owns;
            }
        }

        template<typename _Clock, typename _Duration>
        bool try_lock_until(const std::chrono::time_point<_Clock, _Duration> &atime) {
            if (!_device || _owns)
                chSysHalt("");
            else {
                _owns = _device->try_lock_until(atime);
                return _owns;
            }
        }

        template<typename _Rep, typename _Period>
        bool try_lock_for(const std::chrono::duration<_Rep, _Period> &rtime) {
            if (!_device || _owns)
                chSysHalt("");
            else {
                _owns = _device->try_lock_for(rtime);
                return _owns;
            }
        }

        void unlock() {
            if (!_owns)
                chSysHalt("");
            else if (_device) {
                _device->unlock();
                _owns = false;
            }
        }

        void swap(unique_lock &u) noexcept {
            std::swap(_device, u._device);
            std::swap(_owns, u._owns);
        }

        mutex_type *release() noexcept {
            mutex_type *ret = _device;
            _device = 0;
            _owns = false;
            return ret;
        }

        bool owns_lock() const noexcept { return _owns; }

        explicit operator bool() const noexcept { return owns_lock(); }

        mutex_type *mutex() const noexcept { return _device; }

    private:
        mutex_type *_device;
        bool _owns;
    };

    template<typename _Mutex>
    inline void swap(unique_lock<_Mutex> &x, unique_lock<_Mutex> &y) noexcept { x.swap(y); }
}