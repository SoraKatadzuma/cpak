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

    // When all else fails, use this, who knows what the problem could be..
    unknown = std::numeric_limits<std::uint16_t>::max(),

    // Define beginning and end of generic range.
    generic_begin = EXIT_SUCCESS,
    generic_end   = invalidCPakFile,
};


constexpr std::string_view kSuccessMessage = "Success"; // Generic success.
constexpr std::string_view kFailureMessage = "Failure"; // Generic failure.


}


namespace cpak {


/// @brief  Gets the category for the generic status codes.
/// @return The preconstructed generic category.
const std::error_category& generic_category() noexcept;

/// @brief  Gets the category for the build status codes.
/// @return The preconstructed build category.
const std::error_category& build_category() noexcept;

/// @brief  Gets the category for the unknown status codes.
/// @return The preconstructed unknown category.
const std::error_category& unknown_category() noexcept;


/// @brief  Makes an error code from the given \c errc::values.
/// @param  value The value of the \c cpak::errc code.
/// @return The constructed error code.
inline std::error_code make_error_code(errc::values value) noexcept {
    if (value >= errc::generic_begin && value <= errc::generic_end)
        return std::error_code(value, cpak::generic_category());
    return std::error_code(value, cpak::unknown_category());
}

}