#pragma once
#include "common.hpp"

namespace cpak {


/// @brief   Defines an build specific option for a project.
/// @details Build options are used to configure the build process for a project,
///          or at least that's what it's assumed to be used for. As you may
///          find, they are more flexible than that. They contain a name, value,
///          and a description. The name is used to identify the option, the
///          value is used to configure the build process, and the description
///          is used to describe the option to the user.
/// @remarks When a project is built with options, the options and their values
///          are included in the hash of the project build. This make it
///          possible to have multiple builds of the same project with different
///          options, and they remain cached.
struct BuildOption {
    std::optional<std::string> desc;

    std::string name;
    std::string value;
};


/// @brief Validates the given build option schema.
/// @param node The node containing the build option.
inline static void
validateBuildOptionSchema(const YAML::Node& node) {
    // Validate required fields.
    if (!node["name"])
        throw YAML::Exception(node.Mark(), "Build option is missing a name.");
    else if (!node["name"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Build option name must be a string.");

    if (!node["value"])
        throw YAML::Exception(node.Mark(), "Build option is missing a value.");
    else if (!node["value"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Build option value must be a string.");

    // Validate optional fields.
    if (node["desc"] && !node["desc"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Build option desc must be a string.");
}


/// @brief Interpolates the given argument with the given options.
/// @param argument The argument to interpolate with the options.
/// @param options The options to interpolate with.
inline void
interpolateOptions(std::string& argument,
                   const std::vector<BuildOption>& options) noexcept {
    // Looking to match ${OPTION_NAME}.
    static std::regex optionRegex("\\$\\{([A-Z_]+)\\}");

    // Match all the options in the argument.
    std::smatch match;
    while (std::regex_search(argument, match, optionRegex)) {
        const auto& option = std::find_if(
            options.begin(), options.end(), [&match](const auto& option) {
                return option.name == match[1].str();
            });

        if (option == options.end()) continue;

        argument.replace(match.position(), match.length(), option->value);
    }
}


} // namespace cpak


template<>
struct YAML::convert<cpak::BuildOption> {
    static Node
    encode(const cpak::BuildOption& rhs) {
        Node node;
        node["name"]  = rhs.name;
        node["value"] = rhs.value;
        if (rhs.desc != std::nullopt)
            node["desc"] = *rhs.desc;

        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::BuildOption& rhs) {
        cpak::validateBuildOptionSchema(node);
        rhs.name  = node["name"].as<std::string>();
        rhs.value = node["value"].as<std::string>();
        if (node["desc"])
            rhs.desc = node["desc"].as<std::string>();
        return true;
    }
};