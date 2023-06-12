#pragma once
#include "../common.hpp"

namespace cpak::utilities {


/// @brief  Trims any whitespace from the left side of the input.
/// @param  input The input string to trim whitespace from.
/// @return The trimmed string.
inline std::string
ltrim(std::string input) noexcept {
    return input.substr(input.find_first_not_of(" \t\n\r\f\v"));
}


/// @brief  Trims any whitespace from the right side of the input.
/// @param  input The input string to trim whitespace from.
/// @return The trimmed string.
inline std::string
rtrim(std::string input) noexcept {
    return input.substr(0, input.find_last_not_of(" \t\n\r\f\v") + 1);
}


/// @brief  Trims any whitespace from both sides of the input.
/// @param  input The input string to trim whitespace from.
/// @return The trimmed string.
inline std::string
trim(std::string input) noexcept {
    return ltrim(rtrim(input));
}


/// @brief  Splits a string once using the delimiter.
/// @param  str The string to split.
/// @param  delimiter The delimiter to split the string with.
/// @return A pair containing the two string halves.
inline std::pair<std::string, std::string>
splitString(std::string_view str, std::string_view delimiter = " ") noexcept {
    const auto delimiterPosition = str.find(delimiter);
    if (delimiterPosition == std::string_view::npos)
        return { std::string(str), std::string() };

    return { std::string(str.substr(0, delimiterPosition)),
             std::string(str.substr(delimiterPosition + delimiter.size())) };
}


} // namespace cpak::utilities