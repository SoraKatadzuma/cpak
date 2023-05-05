#pragma once

namespace cpak {


/// @brief   Stores the configuration for the application.
/// @details This struct stores the configuration for the application for a
///          given execution. These values are set by the command line
///          arguments or the application configuration file.
struct Configuration {
    bool verbose{ false };
};


}