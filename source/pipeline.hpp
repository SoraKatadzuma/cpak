#pragma once
#include "cpakfile.hpp"


namespace cpak {


/// @brief  Finds the path to the dependency with the given cpakid.
/// @param  dependency The dependency to find the path to.
/// @return A pair of the path to the dependency and the error code.
std::pair<std::filesystem::path, std::error_code>
findDependencyPath(const Dependency& dependency) noexcept;


/// @brief  Loads the CPak file from the given project path.
/// @param  projectPath The path to the CPak project to load the CPakFile from.
/// @return A pair of the loaded CPakFile and the error code.
/// @note   If the error code is set, the CPakFile will be empty.
std::pair<std::optional<CPakFile>, std::error_code>
loadCPakFile(const std::filesystem::path& projectPath) noexcept;


/// @brief  Queues the given project for building.
/// @param  project The project that needs to be built.
/// @return The status code for the operation.
std::error_code queueForBuild(const CPakFile& project) noexcept;


}