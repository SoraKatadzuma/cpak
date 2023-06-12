#pragma once
#include "configuration.hpp"
#include "project.hpp"


namespace cpak::application {


constexpr std::uint32_t kMajor         = 0;
constexpr std::uint32_t kMinor         = 1;
constexpr std::uint32_t kPatch         = 0;
constexpr std::string_view kPrerelease = "alpha";
constexpr std::string_view kBuildMeta  = "dev";
constexpr std::string_view kTextLogo   = R"(
 ::::::::  :::::::::      :::     :::    :::
:+:    :+: :+:    :+:   :+: :+:   :+:   :+:
+:+        +:+    +:+  +:+   +:+  +:+  +:+
+#+        +#++:++#+  +#++:++#++: +#++:++
+#+        +#+        +#+     +#+ +#+  +#+
#+#    #+# #+#        #+#     #+# #+#   #+#
 ########  ###        ###     ### ###    ###)";


/// @brief  Initializes the application.
/// @return Status code for the application initialization.
std::error_code
init() noexcept;


/// @brief  Runs the application, handling command line arguments.
/// @param  arguments The command line arguments to handle.
/// @return Status code for the application running.
std::error_code
run(const std::vector<std::string>& arguments) noexcept;


} // namespace cpak::application
