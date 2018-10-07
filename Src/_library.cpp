#include "../Include/chstd.hpp"
#include "new.icc"
#include "cxxabi.icc"
#include "hal.h"

namespace chstd {
    void init() noexcept {
        halInit();
        chSysInit();
        chstd::thread([]() {
            for (;;) {
                palTogglePad(GPIOA, GPIOA_LED);
                chstd::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }).detach();
    }

    namespace { struct AA { AA() noexcept { init(); }} _g_init_; }
}
