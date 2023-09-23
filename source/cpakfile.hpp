#pragma once
#include "dependency.hpp"
#include "install.hpp"
#include "project.hpp"
#include "repository.hpp"
#include "target.hpp"
#include "utilities/stropts.hpp"


namespace cpak {

/// @brief   Represents the project file for a CPak project.
/// @details Contains all the project information for a CPak project. This
///          includes the build targets.
struct CPakFile {
    std::optional<Install> install;
    std::vector<BuildTarget> targets;
    std::vector<BuildOption> options;
    std::vector<Repository> repositories;
    std::vector<Dependency> dependencies;

    ProjectInfo project;

    // Not to be serialized.
    // Used during the build process.
    std::filesystem::path projectPath;
    std::filesystem::path buildPath;

    public:
    /// @brief  Gets the logging path for the builds of this project.
    /// @return The logging path for the builds of this project.
    inline std::filesystem::path
    buildLoggingPath() const noexcept {
        return buildPath / "logs";
    }

    /// @brief  Gets the path to the binaries build directory for this project.
    /// @return The path to the binaries build directory for this project.
    inline std::filesystem::path
    binaryBuildPath() const noexcept {
        return buildPath / "binaries";
    }

    /// @brief  Gets the path to the libraries build directory for this project.
    /// @return The path to the libraries build directory for this project.
    inline std::filesystem::path
    libraryBuildPath() const noexcept {
        return buildPath / "libraries";
    }

    /// @brief  Gets the path to the objects build directory for this project.
    /// @return The path to the objects build directory for this project.
    inline std::filesystem::path
    objectBuildPath() const noexcept {
        return buildPath / "objects";
    }

    // TODO: extract to application.
    static std::filesystem::path
    rootInstallPath() noexcept {
    #if _WIN32
        auto installPath = std::filesystem::path(std::getenv("USERPROFILE"));
    #else
        auto installPath = std::filesystem::path(std::getenv("HOME"));
    #endif

        return installPath / ".cpak";
    }


    static std::filesystem::path
    globalInstallPath() noexcept {
        return rootInstallPath() / "global";
    }


    static std::filesystem::path
    localInstallPath() noexcept {
        return rootInstallPath() / "local";
    }

    
    inline std::filesystem::path
    installPath() const noexcept {
        if (install == std::nullopt)
            return localInstallPath();

        return install->global
            ? globalInstallPath()
            : localInstallPath();
    }


    inline std::filesystem::path
    binaryInstallPath() const noexcept {
        return installPath() / "bin";
    }


    inline std::filesystem::path
    libraryInstallPath() const noexcept {
        return installPath() / "lib";
    }


    inline std::filesystem::path
    includeInstallPath() const noexcept {
        return installPath() / "include" / project.name;
    }
};


/// @brief Validates the given CPakFile.
/// @param node The node to validate.
inline void
validateCPakFileSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "CPakFile is not a map");

    if (!node["project"])
        throw YAML::Exception(node.Mark(),
                              "CPakFile must contain project info.");

    if (!node["targets"])
        throw YAML::Exception(node.Mark(),
                              "CPakFile must contain build targets.");
    else if (!node["targets"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "CPakFile targets must be a sequence.");
    else if (node["targets"].size() == 0)
        throw YAML::Exception(node.Mark(),
                              "CPakFile targets must not be empty.");


    if (node["options"] && !node["options"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "CPakFile options must be a sequence.");

    if (node["repositories"] && !node["repositories"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "CPakFile repositories must be a sequence.");

    if (node["dependencies"] && !node["dependencies"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "CPakFile dependencies must be a sequence.");

    
    if (node["install"] && !node["install"].IsMap())
        throw YAML::Exception(node.Mark(), "CPakFile install is not a map.");
}


} // namespace cpak


template<>
struct YAML::convert<cpak::CPakFile> {
    static Node
    encode(const cpak::CPakFile& rhs) {
        Node node;
        node["project"] = rhs.project;
        // Optional fields.
        if (!rhs.options.empty())
            for (const auto& option : rhs.options)
                node["options"].push_back(option);

        if (!rhs.repositories.empty())
            for (const auto& repository : rhs.repositories)
                node["repositories"].push_back(repository);

        if (!rhs.dependencies.empty())
            for (const auto& dependency : rhs.dependencies)
                node["dependencies"].push_back(dependency);

        for (const auto& target : rhs.targets)
            node["targets"].push_back(target);

        if (rhs.install != std::nullopt)
            node["install"] = rhs.install.value();

        return node;
    }

    static bool
    decode(const Node& node, cpak::CPakFile& rhs) {
        cpak::validateCPakFileSchema(node);

        rhs.project = node["project"].as<cpak::ProjectInfo>();
        // Optional fields.
        if (node["options"])
            for (const auto& option : node["options"])
                rhs.options.push_back(option.as<cpak::BuildOption>());

        if (node["repositories"])
            for (const auto& repository : node["repositories"])
                rhs.repositories.push_back(repository.as<cpak::Repository>());

        if (node["dependencies"])
            for (const auto& dependency : node["dependencies"])
                rhs.dependencies.push_back(dependency.as<cpak::Dependency>());

        for (const auto& target : node["targets"])
            rhs.targets.push_back(target.as<cpak::BuildTarget>());

        if (node["install"])
            rhs.install = node["install"].as<cpak::Install>();

        return true;
    }
};