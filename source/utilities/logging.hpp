#pragma once
#include "../common.hpp"

namespace cpak::utilities {


/// @brief  Joins the given string vector into a single string.
/// @param  into The vector to append into.
/// @param  from The vector to append from.
/// @param  pattern The formatting pattern.
inline void
reserveAndAppendFormatted(std::vector<std::string>& into,
                          const std::vector<std::string>& from,
                          const char* pattern = "{}") noexcept {
    if (from.empty()) return;

    // Reserve space incase we need it.
    into.reserve(into.size() + from.size());
    for (const auto& value : from)
        into.emplace_back(fmt::format(fmt::runtime(pattern), value));
}

/// @brief  Gets the name of the log file for the given project.
/// @param  projectName The name of the project to get the log file name for.
/// @return The constructed log file name.
inline std::string
getLogFileName(std::string_view projectName) noexcept {
    std::ostringstream oss;
    auto t  = std::time(nullptr);
    auto tm = std::localtime(&t);

    oss << projectName << "-build." << std::put_time(tm, "%Y%m%d") << ".log";

    return oss.str();
}


} // namespace cpak::utilities