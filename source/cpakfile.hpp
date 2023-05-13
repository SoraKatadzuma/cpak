#pragma once
#include "project.hpp"
#include "target.hpp"
#include "utilities/stropts.hpp"


namespace cpak {

/// @brief   Represents the project file for a CPak project.
/// @details Contains all the project information for a CPak project. This
///          includes the build targets.
struct CPakFile {
    std::vector<BuildTarget> targets;
    std::vector<BuildOption> options;

    ProjectInfo project;
};


/// @brief Validates the given CPakFile.
/// @param node The node to validate.
inline void validateCPakFileSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "CPakFile is not a map");

    if (!node["project"])
        throw YAML::Exception(node.Mark(), "CPakFile must contain project info.");

    if (!node["targets"])
        throw YAML::Exception(node.Mark(), "CPakFile must contain build targets.");
    else if (!node["targets"].IsSequence())
        throw YAML::Exception(node.Mark(), "CPakFile targets must be a sequence.");
    else if (node["targets"].size() == 0)
        throw YAML::Exception(node.Mark(), "CPakFile targets must not be empty.");


    if (node["options"] && !node["options"].IsSequence())
        throw YAML::Exception(node.Mark(), "CPakFile options must be a sequence.");
}


/// @brief Interpolates the given options with the given project.
/// @param project The project to interpolate.
inline void interpolateOptions(CPakFile& project) noexcept {
    for (auto& target : project.targets)
        interpolateOptions(target, project.options);
}


/// @brief Updates the given project options with the given options.
/// @param project The project to update.
/// @param options The options to update the project with.
inline void updateOptions(CPakFile&                 project,
                    const std::vector<std::string>& options) noexcept {
    for (auto&& option : options) {
        auto [optionName, optionValue] =
            utilities::splitString(option, ":");

        auto buildOption = std::find_if(
            project.options.begin(),
            project.options.end(),
            [&optionName](const auto& option) {
                return option.name == optionName;
            }
        );

        std::string value = optionValue;
        if (optionValue.empty()) {
            value = optionName[0] != '!'
                ? std::string("true")
                : std::string("false");
            
            // Strip the ! from the option name.
            optionName.erase(0, 1);
        }

        if (buildOption != project.options.end()) {
            buildOption->value = optionValue;
            continue;
        }

        // Create new option.
        project.options.emplace_back(BuildOption {
            .name  = optionName,
            .value = optionValue,
        });
    }
}


}


template<>
struct YAML::convert<cpak::CPakFile> {
    static Node encode(const cpak::CPakFile& rhs) {
        Node node;
        node["project"] = rhs.project;
        for (const auto& target : rhs.targets)
            node["targets"].push_back(target);

        // Optional fields.
        if (!rhs.options.empty()) {
            for (const auto& option : rhs.options)
                node["options"].push_back(option);
        }

        return node;
    }

    static bool decode(const Node& node, cpak::CPakFile& rhs) {
        cpak::validateCPakFileSchema(node);

        rhs.project = node["project"].as<cpak::ProjectInfo>();
        for (const auto& target : node["targets"])
            rhs.targets.push_back(target.as<cpak::BuildTarget>());

        // Optional fields.
        if (node["options"]) {
            for (const auto& option : node["options"])
                rhs.options.push_back(option.as<cpak::BuildOption>());
        }

        return true;
    }
};