#pragma once
#include "common.hpp"

namespace cpak {


/// @brief   Stores the configuration for the application.
/// @details This struct stores the configuration for the application for a
///          given execution. These values are set by the command line
///          arguments or the application configuration file.
struct Configuration {
    bool verbose{ false };
};


} // namespace cpak


template<>
struct YAML::convert<cpak::Configuration> {
    static Node
    encode(const cpak::Configuration& rhs) {
        Node node;
        return node;
    }

    static bool
    decode(const Node& node, cpak::Configuration& rhs) {
        return true;
    }
};