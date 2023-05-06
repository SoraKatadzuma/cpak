#pragma once
#include "common.hpp"

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
    std::vector<std::string> defines;
    std::vector<std::string> libraries;
    std::vector<std::string> sources;
    std::optional<SearchPaths> search;
    std::optional<std::string> options;

    std::string name;
    TargetType  type;
};


/// @brief  Builds a string representation of the target type.
/// @param  type The target type to build a string representation of.
/// @return The string that represents the target type.
inline std::string buildTypeName(cpak::TargetType type) {
    switch (type) {
    case cpak::TargetType::Undefined:      return "Undefined";
    case cpak::TargetType::Executable:     return "executable";
    case cpak::TargetType::StaticLibrary:  return "static library";
    case cpak::TargetType::DynamicLibrary: return "dynamic library";
    }

    return "Undefined";
}


/// @brief  Builds a target type from a string representation.
/// @param  name The name of the target type to build.
/// @return The target type that was built.
inline TargetType buildTypeFromName(std::string_view name) {
    if (name == "executable") {
        return cpak::TargetType::Executable;
    } else if (name == "static library") {
        return cpak::TargetType::StaticLibrary;
    } else if (name == "dynamic library") {
        return cpak::TargetType::DynamicLibrary;
    }

    return cpak::TargetType::Undefined;
}


/// @brief Validates the schema of the target type.
/// @param node The node containing the target type.
inline void validateTargetTypeSchema(const YAML::Node& node) {
    if (!node.IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a scalar.");
}


/// @brief Validates the schema of the search paths.
/// @param node The node containing the search paths.
inline void validateSearchPathsSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "Search paths must be a map.");
    
    if (node["include"] && !node["include"].IsSequence())
        throw YAML::Exception(node.Mark(), "Include paths must be a sequence.");
    if (node["system"] && !node["system"].IsSequence())
        throw YAML::Exception(node.Mark(), "System paths must be a sequence.");
    if (node["library"] && !node["library"].IsSequence())
        throw YAML::Exception(node.Mark(), "Library paths must be a sequence.");
}


/// @brief Validates the schema of the build target.
/// @param node The node containing the build target.
inline void validateTargetSchema(const YAML::Node& node) {
    // Validate required fields.
    if (!node["name"])
        throw YAML::Exception(node.Mark(), "Target is missing a name.");
    else if (!node["name"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target name must be a string.");

    if (!node["type"])
        throw YAML::Exception(node.Mark(), "Target is missing a type.");
    else if (!node["type"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a string.");

    if (!node["sources"])
        throw YAML::Exception(node.Mark(), "Target is missing sources.");
    else if (!node["sources"].IsSequence())
        throw YAML::Exception(node.Mark(), "Target sources must be a sequence.");
    else if (node["sources"].size() == 0)
        throw YAML::Exception(node.Mark(), "Target sources must not be empty.");

    // Validate optional fields if present.
    if (node["options"] && !node["options"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target options must be a string.");

    if (node["defines"] && !node["defines"].IsSequence())
        throw YAML::Exception(node.Mark(), "Target defines must be a sequence.");

    if (node["libraries"] && !node["libraries"].IsSequence())
        throw YAML::Exception(node.Mark(), "Target libraries must be a sequence.");
}

}


template<>
struct YAML::convert<cpak::TargetType> {
    static Node encode(const cpak::TargetType& rhs) {
        return Node(cpak::buildTypeName(rhs));
    }

    static bool decode(const Node& node, cpak::TargetType& rhs) {
        cpak::validateTargetTypeSchema(node);
        rhs = cpak::buildTypeFromName(node.as<std::string>());
        return true;
    }
};

template<>
struct YAML::convert<cpak::SearchPaths> {
    static Node encode(const cpak::SearchPaths& rhs) {
        Node node;
        node["include"] = rhs.include;
        node["system"]  = rhs.system;
        node["library"] = rhs.library;
        return node;
    }

    static bool decode(const YAML::Node& node, cpak::SearchPaths& rhs) {
        cpak::validateSearchPathsSchema(node);
        if (node["include"]) rhs.include = node["include"].as<std::vector<std::string>>();
        if (node["system"])  rhs.system  = node["system"].as<std::vector<std::string>>();
        if (node["library"]) rhs.library = node["library"].as<std::vector<std::string>>();
        return true;
    }
};

template<>
struct YAML::convert<cpak::BuildTarget> {
    static Node encode(const cpak::BuildTarget& rhs) {
        Node node;
        node["name"]    = rhs.name;
        node["type"]    = rhs.type;
        node["sources"] = rhs.sources;
        
        // Handle optional fields.
        if (rhs.options.has_value())  node["options"]   = *rhs.options;
        if (rhs.search.has_value())   node["search"]    = *rhs.search;
        if (rhs.defines.empty())      node["defines"]   = rhs.defines;
        if (!rhs.libraries.empty())   node["libraries"] = rhs.libraries;
        return node;
    }

    static bool decode(const Node& node, cpak::BuildTarget& rhs) {
        cpak::validateTargetSchema(node);
        rhs.name    = node["name"].as<std::string>();
        rhs.type    = node["type"].as<cpak::TargetType>();
        rhs.sources = node["sources"].as<std::vector<std::string>>();
        if (node["options"])   rhs.options   = node["options"].as<std::string>();
        if (node["defines"])   rhs.defines   = node["defines"].as<std::vector<std::string>>();
        if (node["libraries"]) rhs.libraries = node["libraries"].as<std::vector<std::string>>();
        if (node["search"])    rhs.search    = node["search"].as<cpak::SearchPaths>();
        return true;
    }
};