#pragma once

#include "mutex.hpp"

namespace chstd {
    enum class cv_status {
        no_timeout, timeout
    };

    class condition_variable {
    public:
        void notify_one() noexcept {
            chSysLock();
            if (queue_notempty(&_queue)) {
                chSchWakeupS(queue_fifo_remove(&_queue), MSG_OK);
            }
            chSysUnlock();
        }

        void notify_all() noexcept {
            chSysLock();
            while (queue_notempty(&_queue)) {
                chSchReadyI(queue_fifo_remove(&_queue))->u.rdymsg = MSG_RESET;
            }
            chSchRescheduleS();
            chSysUnlock();
        }

        void wait(unique_lock <mutex> &lk) {
            chSysLock();
            // Releasing "current" mutex.
            chSemSignalI(&lk.mutex()->native_handle());
            // Start waiting on the condition variable, on exit the mutex is taken again.
            currp->u.wtobjp = &_queue;
            queue_prio_insert(currp, &_queue);
            chSchGoSleepS(CH_STATE_WTCOND);
            chSemWaitS(&lk.mutex()->native_handle());
            chSysUnlock();
        }

        template<class Rep, class Period>
        cv_status wait_for(unique_lock <mutex> &lk, const std::chrono::duration<Rep, Period> &d) {
            return wait_for_it(&lk.mutex()->native_handle(),
                               std::chrono::duration_cast<std::chrono::microseconds>(d).count());
        }

        template<class Clock, class Duration>
        cv_status wait_until(unique_lock <mutex> &, const std::chrono::time_point<Clock, Duration> &t) {
            return wait_for(t - Clock::now());
        }

        template<class Predicate>
        void wait(unique_lock <mutex> &lock, Predicate pred) { while (!pred()) wait(lock); }

        template<class Rep, class Period, class Predicate>
        bool wait_for(unique_lock <mutex> &lock, const std::chrono::duration<Rep, Period> &d, Predicate pred) {
            while (!pred())
                if (wait_for(lock, d) == cv_status::timeout)
                    return false;
            return true;
        }

        template<class Clock, class Duration, class Predicate>
        bool
        wait_until(unique_lock <mutex> &lock, const std::chrono::time_point<Clock, Duration> &t, Predicate pred) {
            while (!pred())
                if (wait_until(lock, t) == cv_status::timeout)
                    return false;
            return true;
        }

    private:
        cv_status wait_for_it(semaphore_t *sem, long long int microseconds) noexcept {
            chSysLock();
            // Releasing "current" mutex.
            chSemSignalI(sem);
            // Start waiting on the condition variable, on exit the mutex is take again.
            currp->u.wtobjp = &_queue;
            queue_prio_insert(currp, &_queue);
            auto msg = chSchGoSleepTimeoutS(CH_STATE_WTCOND, TIME_US2I(microseconds));
            chSemWaitS(sem); // Adjusted according to ISO/IEC 14882
            chSysUnlock();
            return (msg == MSG_TIMEOUT ? cv_status::timeout : cv_status::no_timeout);
        }

        _THREADS_QUEUE_DECL(_queue);
    };
}