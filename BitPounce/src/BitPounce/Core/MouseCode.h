#pragma once

#include "BitPounce/Core/Base.h"

namespace BitPounce {

    enum class Mouse : uint16_t {
        #define BP_MOUSE(caps, name) name,
        #include "MouseCodeList.def"
        #undef BP_MOUSE
    };


};

    #define BP_MOUSE(caps, name) \
        static constexpr ::BitPounce::Mouse BP_MOUSE_##caps = ::BitPounce::Mouse::name;
        #include "MouseCodeList.def"
    #undef BP_MOUSE

