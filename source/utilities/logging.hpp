#pragma once

namespace cpak::utilities {


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