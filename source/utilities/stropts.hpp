#pragma once
#include "../common.hpp"

namespace cpak::utilities {


/// @brief  Splits a string once using the delimiter.
/// @param  str The string to split.
/// @param  delimiter The delimiter to split the string with.
/// @return A pair containing the two string halves.
inline std::pair<std::string, std::string>
splitString(std::string_view str, std::string_view delimiter = " ") noexcept {
    const auto delimiterPosition = str.find(delimiter);
    if (delimiterPosition == std::string_view::npos)
        return { std::string(str), std::string() };

    return { 
        std::string(str.substr(0, delimiterPosition)),
        std::string(str.substr(delimiterPosition + delimiter.size()))
    };
}


}