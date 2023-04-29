#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "yaml-cpp/yaml.h"

namespace cpak {


/// @brief Describes the different types of targets.
enum struct TargetType {
    Undefined = -1,
    Executable,
    StaticLibrary,
    DynamicLibrary,
};


/// @brief   Contains the search paths for a given target.
/// @details This is composed of include, system, and library paths. These
///          represent the paths that the compiler will search for headers,
///          system headers, and libraries respectively.
/// @remarks This is based on the options that are available in the GNU
///          toolchain.
struct SearchPaths {
    std::vector<std::string> include;
    std::vector<std::string> system;
    std::vector<std::string> library;
};


/// @brief   Contains the build target information.
/// @details ...
struct BuildTarget {
    SearchPaths search;
    
    std::vector<std::string> defines;
    std::vector<std::string> libraries;
    std::vector<std::string> sources;

    std::string name;
    std::string options;
    TargetType  type;
};


/// @brief  Builds a string representation of the target type.
/// @param  type The target type to build a string representation of.
/// @return The string that represents the target type.
inline std::string buildTypeName(cpak::TargetType type) {
    switch (type) {
    case cpak::TargetType::Undefined:      return "Undefined";
    case cpak::TargetType::Executable:     return "executable";
    case cpak::TargetType::StaticLibrary:  return "static_library";
    case cpak::TargetType::DynamicLibrary: return "dynamic_library";
    }

    return "Undefined";
}


/// @brief  Builds a target type from a string representation.
/// @param  name The name of the target type to build.
/// @return The target type that was built.
inline TargetType buildTypeFromName(std::string_view name) {
    if (name == "executable") {
        return cpak::TargetType::Executable;
    } else if (name == "static_library") {
        return cpak::TargetType::StaticLibrary;
    } else if (name == "dynamic_library") {
        return cpak::TargetType::DynamicLibrary;
    }

    return cpak::TargetType::Undefined;
}


}


template<>
struct YAML::convert<cpak::TargetType> {
    static Node encode(const cpak::TargetType& rhs) {
        return Node(cpak::buildTypeName(rhs));
    }

    static bool decode(const Node& node, cpak::TargetType& rhs) {
        if (!node.IsScalar()) {
            return false;
        }

        rhs = cpak::buildTypeFromName(node.as<std::string>());
        return true;
    }
};

template<>
struct YAML::convert<cpak::BuildTarget> {
    static Node encode(const cpak::BuildTarget& rhs) {
        Node node;
        node["name"]      = rhs.name;
        node["type"]      = rhs.type;
        node["defines"]   = rhs.defines;
        node["options"]   = rhs.options;
        node["libraries"] = rhs.libraries;
        node["sources"]   = rhs.sources;

        node["search"]["include"] = rhs.search.include;
        node["search"]["system"]  = rhs.search.system;
        node["search"]["library"] = rhs.search.library;

        return node;
    }

    static bool decode(const Node& node, cpak::BuildTarget& rhs) {
        if (!node.IsMap())
            return false;

        rhs.name    = node["name"].as<std::string>();
        rhs.type    = node["type"].as<cpak::TargetType>();
        rhs.sources = node["sources"].as<std::vector<std::string>>();
        if (node["options"])
            rhs.options = node["options"].as<std::string>();

        if (node["defines"])
            rhs.defines = node["defines"].as<std::vector<std::string>>();
        
        if (node["libraries"])
            rhs.libraries = node["libraries"].as<std::vector<std::string>>();
        
        auto search = node["search"];
        if (search) {
            if (search["include"])
                rhs.search.include = node["search"]["include"].as<std::vector<std::string>>();
            
            if (search["system"])
                rhs.search.system = node["search"]["system"].as<std::vector<std::string>>();
            
            if (search["library"])
                rhs.search.library = node["search"]["library"].as<std::vector<std::string>>();
        }

        return true;
    }
};