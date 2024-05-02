
#pragma once
#include "Utils.h"

namespace Settings {
    constexpr std::uint32_t kSerializationVersion = 35;
    constexpr std::uint32_t kDataKey = 'STFV';
    static const std::map<std::uint32_t, unsigned int> version_map = {
        {34,1}, 
        {kSerializationVersion, 2}
    };

    static const unsigned int instance_limit = 1000;
};