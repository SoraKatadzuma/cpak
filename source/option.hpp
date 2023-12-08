#pragma once
#include "common.hpp"
#include "utilities/stropts.hpp"

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
interpolateOptions(std::string& evaluated,
                   const std::string& unevaluated,
                   const std::vector<BuildOption>& options) {
    // Looking to match ${OPTION_NAME}.
    static std::regex optionRegex("\\$\\{([A-Z_]+)\\}");

    evaluated = unevaluated;

    // Match all the options in the argument.
    auto match        = std::smatch{};
    auto lastPosition = std::smatch::size_type{0};
    while (std::regex_search(
        unevaluated.begin() + lastPosition,
        unevaluated.end(),
        match,
        optionRegex))
    {
        // this function is on life support
        // it really needs to be rewritten
        const auto& option = std::find_if(
            options.begin(), options.end(), [&match](const auto& option) {
                return option.name == match[1].str();
            });

        if (match.position() >= evaluated.length())
            break;

        if (option != options.end())
            evaluated.replace(match.position(), match.length(), option->value);

        lastPosition += match.length();
        if (lastPosition == match.position())
            break;
    }
}


inline void
interpolateOptions(bool& outEvaluated,
                   const std::string& unevaluated,
                   const std::vector<BuildOption>& options) {
    auto toEvaluate = unevaluated;
    auto position   = std::string::size_type{0};
    while ((position = toEvaluate.rfind("${", 0)) != std::string::npos)
        interpolateOptions(toEvaluate, toEvaluate, options);

    if (toEvaluate == "true" || toEvaluate == "1") {
        outEvaluated = true;
        return;
    }
    
    if (toEvaluate == "false" || toEvaluate == "0") {
        outEvaluated = false;
        return;
    }

    // TODO: throw better exception.
    throw std::runtime_error("Could not evaluate to boolean.");
}


inline std::string
describe(const BuildOption& option) noexcept {
    std::ostringstream oss;
    oss << "\n" << option.name
        << ":"  << option.value
        << " (default)\n";
    if (option.desc != std::nullopt)
        oss << option.desc.value() << "\n";
    return oss.str();
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
            rhs.desc = cpak::utilities::trim(node["desc"].as<std::string>());
        return true;
    }
};