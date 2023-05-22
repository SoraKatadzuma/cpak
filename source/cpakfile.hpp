#pragma once
#include "dependency.hpp"
#include "project.hpp"
#include "repository.hpp"
#include "target.hpp"
#include "utilities/stropts.hpp"


namespace cpak {

/// @brief   Represents the project file for a CPak project.
/// @details Contains all the project information for a CPak project. This
///          includes the build targets.
struct CPakFile {
    std::vector<BuildTarget> targets;
    std::vector<BuildOption> options;
    std::vector<Repository>  repositories;
    std::vector<Dependency>  dependencies;

    ProjectInfo project;

    // Not to be serialized.
    // Used during the build process.
    std::filesystem::path projectPath;
    std::filesystem::path buildPath;

public:
    /// @brief  Gets the logging path for the builds of this project.
    /// @return The logging path for the builds of this project.
    inline std::filesystem::path buildLoggingPath() const noexcept {
        return buildPath / "logs";
    }

    /// @brief  Gets the path to the binaries build directory for this project.
    /// @return The path to the binaries build directory for this project.
    inline std::filesystem::path binariesBuildPath() const noexcept {
        return buildPath / "binaries";
    }

    /// @brief  Gets the path to the libraries build directory for this project.
    /// @return The path to the libraries build directory for this project.
    inline std::filesystem::path librariesBuildPath() const noexcept {
        return buildPath / "libraries";
    }

    /// @brief  Gets the path to the objects build directory for this project.
    /// @return The path to the objects build directory for this project.
    inline std::filesystem::path objectsBuildPath() const noexcept {
        return buildPath / "objects";
    }
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

    if (node["repositories"] && !node["repositories"].IsSequence())
        throw YAML::Exception(node.Mark(), "CPakFile repositories must be a sequence.");

    if (node["dependencies"] && !node["dependencies"].IsSequence())
        throw YAML::Exception(node.Mark(), "CPakFile dependencies must be a sequence.");
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

        if (!rhs.repositories.empty()) {
            for (const auto& repository : rhs.repositories)
                node["repositories"].push_back(repository);
        }

        if (!rhs.dependencies.empty()) {
            for (const auto& dependency : rhs.dependencies)
                node["dependencies"].push_back(dependency);
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

        if (node["repositories"]) {
            for (const auto& repository : node["repositories"])
                rhs.repositories.push_back(repository.as<cpak::Repository>());
        }

        if (node["dependencies"]) {
            for (const auto& dependency : node["dependencies"])
                rhs.dependencies.push_back(dependency.as<cpak::Dependency>());
        }

        return true;
    }
};