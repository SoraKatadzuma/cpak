#pragma once
#include "option.hpp"
#include "property.hpp"

namespace cpak {


/// @brief Describes the different types of targets.
enum struct TargetType {
    Undefined = -1,
    Executable,
    StaticLibrary,
    DynamicLibrary,
    Interface,
};


/// @brief   Contains the search paths for a given target.
/// @details This is composed of include, system, and library paths. These
///          represent the paths that the compiler will search for headers,
///          system headers, and libraries respectively.
/// @remarks This is based on the options that are available in the GNU
///          toolchain.
struct SearchPaths {
    VectorProperty<std::string> include;
    VectorProperty<std::string> system;
    VectorProperty<std::string> library;
};


/// @brief   Contains the build target information.
/// @details ...
struct BuildTarget {
    VectorProperty<std::string> defines;
    VectorProperty<std::string> interfaces;
    VectorProperty<std::string> libraries;
    VectorProperty<std::string> sources;
    VectorProperty<std::string> options;
    OptionalProperty<SearchPaths> search;
    RequiredProperty<std::string> name{ "INVALID" };
    RequiredProperty<TargetType> type{ TargetType::Undefined };
};


/// @brief   Contains the install target information.
/// @details ...
struct InstallTarget {
    RequiredProperty<std::string> source;
    RequiredProperty<std::string> destination;
};


/// @brief  Builds a string representation of the target type.
/// @param  type The target type to build a string representation of.
/// @return The string that represents the target type.
inline std::string
buildTypeName(TargetType type) {
    switch (type) {
    case TargetType::Undefined: return "Undefined";
    case TargetType::Executable: return "executable";
    case TargetType::StaticLibrary: return "static library";
    case TargetType::DynamicLibrary: return "dynamic library";
    case TargetType::Interface: return "interface";
    }

    return "INVALID";
}


/// @brief  Builds a target type from a string representation.
/// @param  name The name of the target type to build.
/// @return The target type that was built.
inline TargetType
buildTypeFromName(std::string_view name) {
    if (name == "executable") return cpak::TargetType::Executable;
    else if (name == "static library") return cpak::TargetType::StaticLibrary;
    else if (name == "dynamic library") return cpak::TargetType::DynamicLibrary;
    else if (name == "interface") return cpak::TargetType::Interface;

    return cpak::TargetType::Undefined;
}


/// @brief Validates the schema of the target type.
/// @param node The node containing the target type.
inline void
validateTargetTypeSchema(const YAML::Node& node) {
    if (!node.IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a scalar.");
}


/// @brief Validates the schema of the search paths.
/// @param node The node containing the search paths.
inline void
validateSearchPathsSchema(const YAML::Node& node) {
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
inline void
validateTargetSchema(const YAML::Node& node) {
    // Validate required fields.
    if (!node["name"])
        throw YAML::Exception(node.Mark(), "Target is missing a name.");
    else if (!node["name"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target name must be a string.");

    if (!node["type"])
        throw YAML::Exception(node.Mark(), "Target is missing a type.");
    else if (!node["type"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a string.");

    // Because the target could be a interface, and interfaces don't require
    // sources, we have to defer the check for absence of sources until after
    // we know the target type. The checks within this block are still valid
    // for interfaces.
    if (node["sources"]) {
        if (!node["sources"].IsSequence())
            throw YAML::Exception(node.Mark(),
                                  "Target sources must be a sequence.");
        else if (node["sources"].size() == 0)
            throw YAML::Exception(node.Mark(),
                                  "Target sources must not be empty.");
    }

    // Validate optional fields if present.
    if (node["defines"] && !node["defines"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target defines must be a sequence.");

    if (node["interfaces"] && !node["interfaces"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target interfaces must be a sequence.");

    if (node["libraries"] && !node["libraries"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target libraries must be a sequence.");

    if (node["options"] && !node["options"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target options must be a string.");
}


/// @brief  Converts the given target to a string.
/// @param  target The target to convert to a string.
/// @return The string representation of the target.
inline std::string
to_string(const BuildTarget& target) noexcept {
    std::ostringstream oss;
    oss << *target.name << " (" << buildTypeName(*target.type) << ") {\n";
    oss << "    Defines: " << vectorPropertyToString(target.defines, ';')
        << "\n";
    oss << "    Interfaces: " << vectorPropertyToString(target.interfaces, ';')
        << "\n";
    oss << "    Libraries: " << vectorPropertyToString(target.libraries, ';')
        << "\n";
    oss << "    Sources: " << vectorPropertyToString(target.sources, ';')
        << "\n";
    oss << "    Options: "
        << vectorPropertyToString(target.options); // hasn't been trimmed yet.

    if (target.search != std::nullopt) {
        oss << "    Search: {\n";
        oss << "        Include: "
            << vectorPropertyToString(target.search->include, ';') << "\n";
        oss << "        System: "
            << vectorPropertyToString(target.search->system, ';') << "\n";
        oss << "        Library: "
            << vectorPropertyToString(target.search->library, ';') << "\n";
        oss << "    }\n";
    }

    oss << "}";
    return oss.str();
}


} // namespace cpak


template<>
struct YAML::convert<cpak::RequiredProperty<cpak::TargetType>> {
    static Node
    encode(const cpak::RequiredProperty<cpak::TargetType>& rhs) {
        return Node(cpak::buildTypeName(*rhs));
    }

    static bool
    decode(const Node& node, cpak::RequiredProperty<cpak::TargetType>& rhs) {
        cpak::validateTargetTypeSchema(node);
        rhs = cpak::buildTypeFromName(node.as<std::string>());
        return true;
    }
};

template<>
struct YAML::convert<cpak::OptionalProperty<cpak::SearchPaths>> {
    static Node
    encode(const cpak::OptionalProperty<cpak::SearchPaths>& rhs) {
        return rhs.has_value() ? Node(*rhs) : Node();
    }

    static bool
    decode(const Node& node, cpak::OptionalProperty<cpak::SearchPaths>& rhs) {
        cpak::validateSearchPathsSchema(node);
        rhs = node.as<cpak::SearchPaths>();
        return true;
    }
};

template<>
struct YAML::convert<cpak::SearchPaths> {
    static Node
    encode(const cpak::SearchPaths& rhs) {
        Node node;
        node["include"] = rhs.include;
        node["system"]  = rhs.system;
        node["library"] = rhs.library;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::SearchPaths& rhs) {
        cpak::validateSearchPathsSchema(node);
        if (node["include"])
            rhs.include = node["include"].as<std::vector<std::string>>();
        if (node["system"])
            rhs.system = node["system"].as<std::vector<std::string>>();
        if (node["library"])
            rhs.library = node["library"].as<std::vector<std::string>>();
        return true;
    }
};

template<>
struct YAML::convert<cpak::BuildTarget> {
    static Node
    encode(const cpak::BuildTarget& rhs) {
        using namespace cpak;

        Node node;
        node["name"]    = rhs.name;
        node["type"]    = rhs.type;
        node["sources"] = rhs.sources;

        // Handle optional fields.
        if (rhs.search != std::nullopt) node["search"] = rhs.search;
        if (rhs.options != std::nullopt)
            node["options"] = vectorPropertyToString(rhs.options);
        if (rhs.defines != std::nullopt) node["defines"] = rhs.defines;
        if (rhs.libraries != std::nullopt) node["libraries"] = rhs.libraries;
        if (rhs.interfaces != std::nullopt) node["interfaces"] = rhs.interfaces;
        return node;
    }

    static bool
    decode(const Node& node, cpak::BuildTarget& rhs) {
        using namespace cpak;

        validateTargetSchema(node);
        rhs.name = node["name"].as<RequiredProperty<std::string>>();
        rhs.type = node["type"].as<RequiredProperty<TargetType>>();

        // Check for sources if this isn't an interface target.
        if (*rhs.type != TargetType::Interface) {
            if (!node["sources"])
                throw YAML::Exception(node.Mark(),
                                      "Target is missing sources.");
            rhs.sources = node["sources"].as<VectorProperty<std::string>>();
        }

        // Handle optional fields.
        if (node["search"])
            rhs.search = node["search"].as<OptionalProperty<SearchPaths>>();
        if (node["options"])
            rhs.options =
                vectorPropertyFromString(node["options"].as<std::string>());
        if (node["defines"])
            rhs.defines = node["defines"].as<VectorProperty<std::string>>();
        if (node["libraries"])
            rhs.libraries = node["libraries"].as<VectorProperty<std::string>>();
        if (node["interfaces"])
            rhs.interfaces =
                node["interfaces"].as<VectorProperty<std::string>>();
        return true;
    }
};