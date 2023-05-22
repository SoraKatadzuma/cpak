#pragma once
#include "identity.hpp"
#include "utilities/noncopyable.hpp"

namespace cpak {


/// @brief   Stores the project information.
/// @details This struct stores the project information for a CPak project.
///          It is an extension of a project identity, containing the paths
///          to the project and build directories, as well as other metadata.
struct ProjectInfo : public Identity {
    // Optional fields.
    std::vector<std::string> authors;
    std::optional<std::string> description;
    std::optional<std::string> license;
    std::optional<std::string> homePage;
    std::optional<std::string> issuesPage;
};



static std::filesystem::path absoluteRootPath(const ProjectInfo& project) noexcept;
static std::filesystem::path absoluteBuildPath(const ProjectInfo& project) noexcept;
static std::filesystem::path absoluteBinariesPath(const ProjectInfo& project) noexcept;
static std::filesystem::path absoluteLibrariesPath(const ProjectInfo& project) noexcept;
static std::filesystem::path absoluteObjectsPath(const ProjectInfo& project) noexcept;

static std::filesystem::path relativeRootPath(const ProjectInfo& project) noexcept;
static std::filesystem::path relativeBuildPath(const ProjectInfo& project) noexcept;
static std::filesystem::path relativeBinariesPath(const ProjectInfo& project) noexcept;
static std::filesystem::path relativeLibrariesPath(const ProjectInfo& project) noexcept;
static std::filesystem::path relativeObjectsPath(const ProjectInfo& project) noexcept;


/// @brief  Validates the given project schema.
/// @param  node The node to validate.
inline void validateProjectSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "Project is not a map");
    
    // Validate required fields.
    if (!node["name"])
        throw YAML::Exception(node.Mark(), "Project is missing a name.");
    else if (!node["name"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project name must be a string.");

    if (!node["gpid"])
        throw YAML::Exception(node.Mark(), "Project is missing a gpid.");
    else if (!node["gpid"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project gpid must be a string.");

    if (!node["semv"])
        throw YAML::Exception(node.Mark(), "Project is missing a semv.");
    else if (!node["semv"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project semv must be a string.");

    // Validate optional fields.        
    if (node["desc"] && !node["desc"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project desc must be a string.");

    if (node["home"] && !node["home"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project home must be a string.");

    if (node["issues"] && !node["issues"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project issues must be a string.");

    if (node["license"] && !node["license"].IsScalar())
        throw YAML::Exception(node.Mark(), "Project license must be a string.");
 
    if (node["authors"] && !node["authors"].IsSequence())
        throw YAML::Exception(node.Mark(), "Project authors must be a sequence.");
}


}


template<>
struct YAML::convert<cpak::ProjectInfo> {
    static Node encode(const cpak::ProjectInfo& rhs) {
        Node node;
        node["name"] = rhs.name;
        node["gpid"] = rhs.gpid;
        node["semv"] = rhs.semv;

        // Optional fields.
        if (rhs.description.has_value()) node["desc"]    = *rhs.description;
        if (rhs.homePage.has_value())    node["home"]    = *rhs.homePage;
        if (rhs.issuesPage.has_value())  node["issues"]  = *rhs.issuesPage;
        if (rhs.license.has_value())     node["license"] = *rhs.license;
        if (!rhs.authors.empty())        node["authors"] = rhs.authors;
        return node;
    }

    static bool decode(const Node& node, cpak::ProjectInfo& rhs) {
        cpak::validateProjectSchema(node);
        rhs.name = node["name"].as<std::string>();
        rhs.gpid = node["gpid"].as<std::string>();
        rhs.semv = node["semv"].as<cpak::version>();
        
        // Optional fields.
        if (node["desc"])    rhs.description = node["desc"].as<std::string>();
        if (node["home"])    rhs.homePage    = node["home"].as<std::string>();
        if (node["issues"])  rhs.issuesPage  = node["issues"].as<std::string>();
        if (node["license"]) rhs.license     = node["license"].as<std::string>();
        if (node["authors"]) rhs.authors     = node["authors"].as<std::vector<std::string>>();
        return true;
    }
};