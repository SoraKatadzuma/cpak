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


/// @brief Validates the given CPakFile.
/// @param node The node to validate.
inline void validateCPakFileSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "CPakFile is not a map");
    
    if (!node["targets"])
        throw YAML::Exception(node.Mark(), "CPakFile does not contain targets.");
    else if (!node["targets"].IsSequence())
        throw YAML::Exception(node.Mark(), "CPakFile targets is not a sequence.");
    else if (node["targets"].size() == 0)
        throw YAML::Exception(node.Mark(), "CPakFile targets is empty.");
}


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
        cpak::validateCPakFileSchema(node);
        for (const auto& target : node["targets"])
            rhs.targets.push_back(target.as<cpak::BuildTarget>());
        
        return true;
    }
};