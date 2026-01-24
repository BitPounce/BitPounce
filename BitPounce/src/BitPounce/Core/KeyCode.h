#pragma once

#include "BitPounce/Core/Base.h"

namespace BitPounce {

    enum class Key : uint16_t {
        #define BP_KEY(caps, name) name,
        #include "KeyList.def"
        #undef BP_KEY
    };


};

    #define BP_KEY(caps, name) \
        static constexpr ::BitPounce::Key BP_KEY_##caps = ::BitPounce::Key::name;
        #include "KeyList.def"
    #undef BP_KEY


namespace BitPounce
{
    Key PlatformKeyToKey(uint32_t key);
    uint32_t KeyToPlatformKey(Key key);

}