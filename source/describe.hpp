#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <sstream>
#include "utilities/stropts.hpp"

namespace cpak {


enum struct DescribeProperty : std::uint8_t {
    Options      = 1 << 0,
    Targets      = 1 << 1,
    All          = 0xFF
};


constexpr std::array<std::string_view, 3> kPropertyNames {
    "options", "targets", "all"
};


inline std::string
describePropertiesToString(std::uint8_t properties) {
    if (properties == static_cast<std::uint8_t>(DescribeProperty::All))
        return std::string(kPropertyNames[2]);

    std::ostringstream oss;
    for (auto index = 0u; index < kPropertyNames.size() - 1; index++) {
        auto positiveProperty = properties & (1 << index);
        if (positiveProperty == 0) continue;
        
        oss << kPropertyNames[index] << ',';
    }
}


inline std::uint8_t
stringToDescribeProperties(std::string_view str) {
    static std::unordered_map<std::string_view, std::uint8_t> kMappedProperties {
        { kPropertyNames[0], static_cast<std::uint8_t>(DescribeProperty::Options)      },
        { kPropertyNames[1], static_cast<std::uint8_t>(DescribeProperty::Targets)      },
        { kPropertyNames[2], static_cast<std::uint8_t>(DescribeProperty::All)          }
    };

    const auto& properties = utilities::splitString(str, ",");
    if (properties.size() == 1 && properties[0] == kPropertyNames[2])
        return static_cast<std::uint8_t>(DescribeProperty::All);

    std::uint8_t result;
    for (const auto& property : properties) {
        const auto& itr = kMappedProperties.find(property);
        if (itr == kMappedProperties.end())
            continue;

        result |= itr->second;
    }

    return result;
}


} // namespace cpak