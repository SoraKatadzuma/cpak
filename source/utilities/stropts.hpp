#pragma once
#include "../common.hpp"
#include <string>

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


/// @brief  Pads the input string to the right with the given
///         padding character.
/// @param  input The input string to pad.
/// @param  length The length to pad the string to.
/// @param  padding The character to pad the string with.
/// @return The padded string.
inline std::string
lpad(std::string input, std::size_t length, char padding = ' ') noexcept {
    input.insert(input.begin(), length - input.size(), padding);
    return input;
}


/// @brief  Pads the input string to the left with the given
///         padding character.
/// @param  input The input string to pad.
/// @param  length The length to pad the string to.
/// @param  padding The character to pad the string with.
/// @return The padded string.
inline std::string
rpad(std::string input, std::size_t length, char padding = ' ') noexcept {
    input.insert(input.end(), length - input.size(), padding);
    return input;
}


/// @brief  Splits a string once using the delimiter.
/// @param  str The string to split.
/// @param  delimiter The delimiter to split the string with.
/// @return A pair containing the two string halves.
inline std::pair<std::string, std::string>
splitStringOnce(std::string_view str,
                std::string_view delimiter = " ") noexcept {
    const auto delimiterPosition = str.find(delimiter);
    if (delimiterPosition == std::string_view::npos)
        return { std::string(str), std::string() };

    return { std::string(str.substr(0, delimiterPosition)),
             std::string(str.substr(delimiterPosition + delimiter.size())) };
}


/// @brief  Splits a string multiple times using the delimiter.
/// @param  str The string to split.
/// @param  delimiter The delimeter to split the string with.
/// @return A vector containing the string parts.
inline std::vector<std::string_view>
splitString(std::string_view str,
            std::string_view delimiter = " ") noexcept {
    std::vector<std::string_view> result;
    
    std::size_t current, previous = 0;
    current = str.find(delimiter);
    while (current != std::string_view::npos) {
        result.emplace_back(str.substr(previous, current - previous));
        previous = current + delimiter.size();
        current  = str.find(delimiter, previous);
    }

    result.emplace_back(str.substr(previous, current - previous));
    return result;
}


inline std::string
textWrap(std::string      source,
         std::size_t      length,
         std::string_view padding = "") noexcept {
    std::string::size_type index = 0;
    do {
        index = source.find(' ', index + length);
        if (index == std::string::npos)
            break;
        source.at(index) = '\n';
        if (padding.size() != 0)
            source.insert(index + 1, padding);
    } while (true);

    return source;
}


} // namespace cpak::utilities