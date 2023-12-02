#pragma once
#include "accessible.hpp"
#include "option.hpp"
#include "utilities/stropts.hpp"
#include <iomanip>
#include <optional>

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
    Accessibles<std::string> include;
    Accessibles<std::string> system;
    Accessibles<std::string> library;
};


/// @brief   Contains the build target information.
/// @details ...
struct BuildTarget {
    Accessibles<std::string> defines;
    Accessibles<std::string> interfaces;
    Accessibles<std::string> libraries;
    Accessibles<std::string> sources;
    Accessibles<std::string> options;
    std::optional<SearchPaths> search;
    std::optional<std::string> desc;

    std::string name{ "INVALID" };
    TargetType  type{ TargetType::Undefined };
};


/// @brief   Contains the install target information.
/// @details ...
struct InstallTarget {
    std::string source;
    std::string destination;
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

    if (node["desc"] && !node["desc"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target description must be a string."); 

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

    if (node["options"] &&
       !node["options"].IsScalar() &&
       !node["options"].IsSequence())
        throw YAML::Exception(node.Mark(), "Target options must be a string or sequence.");
}


/// @brief  Creates a vector from a string.
/// @param  level The access level to assign to the vector.
/// @param  value The string to create the vector from.
/// @param  delimiter How to split the string into values.
/// @return The created vector.
inline Accessibles<std::string>
accessiblesFromString(AccessLevel level,
                      std::string_view value,
                      char delimiter = ' ') noexcept {
    // Split the string into vector of strings, then assign.
    Accessibles<std::string> values;
    std::istringstream iss(value.data());
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        values.push_back(Accessible{
            .stored = token,
            .level = level,
        });
    }

    return values;
}


/// @brief  Creates a string from a vector of accessibles.
/// @param  accessibles The vector to create the string from.
/// @param  delimiter How to join the values into a string.
/// @return The created string.
inline std::string
accessiblesToString(const Accessibles<std::string>& accessibles,
                    char delimiter = ' ') noexcept {
    std::ostringstream oss;
    for (const auto& value : accessibles)
        oss << value.stored << delimiter;

    return oss.str();
}


inline void
assignTargetToAccessibles(Accessibles<std::string>& accessibles,
                          BuildTarget* target) noexcept {
    for (auto& accessible : accessibles)
        accessible.owner = target;
}


/// @brief  Converts the given target to a string.
/// @param  target The target to convert to a string.
/// @return The string representation of the target.
inline std::string
to_string(const BuildTarget& target) noexcept {
    std::ostringstream oss;
    oss << target.name << " (" << buildTypeName(target.type) << ") {\n";
    oss << "    Defines: " << accessiblesToString(target.defines, ';')
        << "\n";
    oss << "    Interfaces: " << accessiblesToString(target.interfaces, ';')
        << "\n";
    oss << "    Libraries: " << accessiblesToString(target.libraries, ';')
        << "\n";
    oss << "    Sources: " << accessiblesToString(target.sources, ';')
        << "\n";
    oss << "    Options: "
        << accessiblesToString(target.options); // hasn't been trimmed yet.

    if (target.search != std::nullopt) {
        oss << "    Search: {\n";
        oss << "        Include: "
            << accessiblesToString(target.search->include, ';') << "\n";
        oss << "        System: "
            << accessiblesToString(target.search->system, ';') << "\n";
        oss << "        Library: "
            << accessiblesToString(target.search->library, ';') << "\n";
        oss << "    }\n";
    }

    oss << "}";
    return oss.str();
}


inline std::string
describe(const BuildTarget& target) noexcept {
    std::ostringstream oss;
    oss << "\n" << target.name
        << " (" << buildTypeName(target.type)
        << ")\n\n";

    if (target.desc != std::nullopt)
        oss << "Description:\n  "
            << utilities::textWrap(target.desc.value(), 50, "  ")
            << "\n";

    if (target.interfaces.size() != 0) {
        oss << "Inherits from:\n";
        for (const auto& interface : target.interfaces)
            oss << " - " << interface.stored << "\n";
        oss << std::endl;
    }

    oss << "Compiled with:\n";
    if (target.search != std::nullopt) {
        for (const auto& sysInclude : target.search->system)
            oss << "-isystem " << sysInclude.stored << "\n";

        for (const auto& regInclude : target.search->include)
            oss << "-I " << regInclude.stored << "\n";

        for (const auto& libInclude : target.search->library)
            oss << "-L " << libInclude.stored << "\n";
    }

    for (const auto& define : target.defines)
        oss << "-D " << define.stored << "\n";

    if (target.options.size() != 0) {
        for (const auto& option : target.options)
            oss << option.stored << " ";
        oss << "\n";
    }

    for (const auto& library : target.libraries)
        oss << "-l " << library.stored << "\n";
    
    oss << "-c\n";
    for (const auto& source : target.sources)
        oss << "  " << source.stored << "\n";

    return oss.str();
}


} // namespace cpak


template<>
struct YAML::convert<cpak::TargetType> {
    static Node
    encode(const cpak::TargetType& rhs) {
        Node node;
        node = cpak::buildTypeName(rhs);
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::TargetType& rhs) {
        cpak::validateTargetTypeSchema(node);
        rhs = cpak::buildTypeFromName(node.as<std::string>());
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
        cpak::SearchPaths paths;
        if (node["include"])
            paths.include = node["include"].as<cpak::Accessibles<std::string>>();
        if (node["system"])
            paths.system = node["system"].as<cpak::Accessibles<std::string>>();
        if (node["library"])
            paths.library = node["library"].as<cpak::Accessibles<std::string>>();
        
        rhs = paths;
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

        if (rhs.desc != std::nullopt)
            node["desc"] = rhs.desc.value();

        // Handle optional fields.
        if (rhs.search != std::nullopt)
            node["search"] = rhs.search.value();
        
        if (!rhs.options.empty()) {
            bool isAllSameAccess;
            auto accessLevel = rhs.options[0].level;
            for (const auto& option : rhs.options) {
                if (option.level != accessLevel) {
                    isAllSameAccess = false;
                    break;
                }
            }

            if (isAllSameAccess) {
                // TODO: convert to function.
                const auto tagName =
                    accessLevel == AccessLevel::eProtected
                        ? "!protected"
                        : accessLevel == AccessLevel::ePrivate
                            ? "!private"
                            : "!public";
                node["options"] = accessiblesToString(rhs.options);
                node.SetTag(tagName);
            } else node["options"] = rhs.options;
        }

        if (!rhs.defines.empty()) node["defines"] = rhs.defines;
        if (!rhs.libraries.empty()) node["libraries"] = rhs.libraries;
        if (!rhs.interfaces.empty()) node["interfaces"] = rhs.interfaces;
        return node;
    }

    static bool
    decode(const Node& node, cpak::BuildTarget& rhs) {
        using namespace cpak;

        validateTargetSchema(node);
        rhs.name = node["name"].as<std::string>();
        rhs.type = node["type"].as<TargetType>();

        if (node["desc"])
            rhs.desc = node["desc"].as<std::string>();

        // Check for sources if this isn't an interface target.
        auto* target = &rhs;
        if (rhs.type != TargetType::Interface) {
            if (!node["sources"])
                throw YAML::Exception(node.Mark(),
                                      "Target is missing sources.");
            rhs.sources = node["sources"].as<cpak::Accessibles<std::string>>();
            assignTargetToAccessibles(rhs.sources, target);
        }

        // Handle optional fields.
        if (node["search"]) {
            rhs.search = node["search"].as<SearchPaths>();
            assignTargetToAccessibles(rhs.search->include, target);
            assignTargetToAccessibles(rhs.search->system, target);
            assignTargetToAccessibles(rhs.search->library, target);
        }

        if (node["options"]) {
            if (node["options"].IsScalar()) {
                AccessLevel level;
                const auto tag = node["options"].Tag();
                if (tag == "!protected")
                    level = AccessLevel::eProtected;
                else if (tag == "!private")
                    level = AccessLevel::ePrivate;
                else
                    level = AccessLevel::ePublic;

                const auto& opts = cpak::utilities::trim(node["options"].as<std::string>());
                rhs.options = accessiblesFromString(level, opts);
                assignTargetToAccessibles(rhs.options, target);
            } else if (node["options"].IsSequence()) {
                rhs.options = node["options"].as<cpak::Accessibles<std::string>>();
                assignTargetToAccessibles(rhs.options, target);
            }
        }

        if (node["defines"]) {
            rhs.defines = node["defines"].as<cpak::Accessibles<std::string>>();
            assignTargetToAccessibles(rhs.defines, target);
        }

        if (node["libraries"]) {
            rhs.libraries = node["libraries"].as<cpak::Accessibles<std::string>>();
            assignTargetToAccessibles(rhs.libraries, target);
        }

        if (node["interfaces"]) {
            rhs.interfaces = node["interfaces"].as<cpak::Accessibles<std::string>>();
            assignTargetToAccessibles(rhs.interfaces, target);
        }

        return true;
    }
};