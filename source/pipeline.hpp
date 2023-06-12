#pragma once
#include "cpakfile.hpp"


namespace cpak {


/// @brief  Queues the given project for building.
/// @param  cpakfile The project that needs to be built.
/// @return The status code for the operation.
std::error_code
queueForBuild(const CPakFile& cpakfile) noexcept;


/// @brief  Queues the given target for building.
/// @param  cpakfile The parent CPakFile the target belongs to.
/// @param  target The target that will be queued for building.
/// @return The status code for the operation.
std::error_code
queueForBuild(const CPakFile& cpakfile, const BuildTarget& target) noexcept;


/// @brief  Executes the build queue.
/// @return The status code for the operation.
std::error_code
executeBuild() noexcept;


} // namespace cpak