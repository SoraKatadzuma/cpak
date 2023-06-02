#pragma once
#include "cpakfile.hpp"

namespace cpak::management {


std::tuple<std::optional<CPakFile>, std::error_code>
loadCPakFile(const std::filesystem::path& projectPath) noexcept;



std::tuple<std::optional<CPakFile>, std::error_code>
loadCPakFile(const std::filesystem::path& projectPath,
             const std::vector<std::string>& options) noexcept;


std::tuple<std::optional<CPakFile>, std::error_code>
loadDependency(const Dependency& dependency) noexcept;


std::tuple<std::optional<CPakFile>, std::error_code>
cloneDependency(const Dependency& dependency,
                const std::string& dependencyPath) noexcept;


std::tuple<std::filesystem::path, std::error_code>
findDependencyPath(const Dependency& dependency) noexcept;


}