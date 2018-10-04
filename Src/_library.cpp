#include "new.icc"
#include "cxxabi.icc"
#include "hal.h"

namespace chstd {
    void init() noexcept {
        halInit();
        chSysInit();
    }

    namespace { struct AA { AA() noexcept { init(); }} _g_init_; }
}
