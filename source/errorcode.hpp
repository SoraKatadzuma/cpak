#pragma once
#include <system_error>


namespace cpak::errc {


/// @brief   The error codes for the application.
/// @details These error codes are used to indicate the status of the
///          application. They will be provided as a std::error_code to the
///          caller. These encompass all errors, including build errors.
enum values : std::uint16_t {
    // Generic error codes.
    success = EXIT_SUCCESS,
    failure = EXIT_FAILURE,
    pathDoesNotExist,
    noCPakFileAtPath,
    invalidCPakFile,

    // Build error codes.
    dependencyNotFound,
    gitRemoteNotFound,
    gitRemoteVersionNotFound,
    gitCloneFailed,
    interfaceNotFound,
    interfaceNameCollision,

    // When all else fails, use this, who knows what the problem could be..
    unknown = std::numeric_limits<std::uint16_t>::max(),

    // Define beginning and end of generic range.
    generic_begin = success,
    generic_end   = invalidCPakFile,

    // Define beginning and end of build range.
    build_begin = dependencyNotFound,
    build_end   = interfaceNameCollision,
};


constexpr std::string_view kSuccessMessage = "Success"; // Generic success.
constexpr std::string_view kFailureMessage = "Failure"; // Generic failure.
constexpr std::string_view kPathDoesNotExistMessage = "Path does not exist";
constexpr std::string_view kNoCPakFileAtPathMessage = "No CPakfile at path";
constexpr std::string_view kInvalidCPakFileMessage  = "Invalid CPakfile";

constexpr std::string_view kDependencyNotFoundMessage = "Dependency not found";
constexpr std::string_view kGitRemoteNotFoundMessage = "Git remote not found";
constexpr std::string_view kGitRemoteVersionNotFoundMessage = "Git remote version not found";
constexpr std::string_view kGitCloneFailedMessage = "Git clone failed";
constexpr std::string_view kInterfaceNotFoundMessage = "Interface not found";
constexpr std::string_view kInterfaceNameCollisionMessage = "Interface name collision";

} // namespace cpak::errc


namespace cpak {


/// @brief  Gets the category for the generic status codes.
/// @return The preconstructed generic category.
const std::error_category&
generic_category() noexcept;

/// @brief  Gets the category for the build status codes.
/// @return The preconstructed build category.
const std::error_category&
build_category() noexcept;

/// @brief  Gets the category for the unknown status codes.
/// @return The preconstructed unknown category.
const std::error_category&
unknown_category() noexcept;


/// @brief  Makes an error code from the given \c errc::values.
/// @param  value The value of the \c cpak::errc code.
/// @return The constructed error code.
inline std::error_code
make_error_code(errc::values value) noexcept {
    if (value >= errc::generic_begin && value <= errc::generic_end)
        return std::error_code(value, cpak::generic_category());
    if (value >= errc::build_begin && value <= errc::build_end)
        return std::error_code(value, cpak::build_category());
    return std::error_code(value, cpak::unknown_category());
}

} // namespace cpak