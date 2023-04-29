#pragma once
#include <iostream>
#include "target.hpp"


namespace cpak {

/// @brief   Represents the project file for a CPak project.
/// @details Contains all the project information for a CPak project. This
///          includes the build targets.
struct CPakFile {
    std::vector<BuildTarget> targets;
};


}


template<>
struct YAML::convert<cpak::CPakFile> {
    static Node encode(const cpak::CPakFile& rhs) {
        Node node;
        for (const auto& target : rhs.targets)
            node["targets"].push_back(target);

        return node;
    }

    static bool decode(const Node& node, cpak::CPakFile& rhs) {
        if (!node.IsMap())
            return false;

        for (const auto& target : node["targets"])
            rhs.targets.push_back(target.as<cpak::BuildTarget>());
        
        return true;
    }
};