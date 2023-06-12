#pragma once

namespace cpak::util {

/// @brief A class that prevents derived classes from being copied.
class NonCopyable {
    protected:
    constexpr NonCopyable() = default;
    virtual ~NonCopyable()  = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable&
    operator=(const NonCopyable&) = delete;
};

} // namespace cpak::util