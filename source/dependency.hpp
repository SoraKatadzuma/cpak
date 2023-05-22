#pragma once
#include "identity.hpp"
#include "repository.hpp"

namespace cpak {


/// @brief   Contains information for pulling a dependency from a repository.
/// @details This struct contains the necessary information to get a 
struct Dependency : public Identity {
    OptionalProperty<Repository> remote;
};


/// @brief Validates the given dependency schema.
/// @param node The node containing the dependency.
inline void validateDependencySchema(const YAML::Node& node) {
    if (node.IsMap()) {
        // Validate required fields.
        if (!node["cpakid"])
            throw YAML::Exception(node.Mark(), "Dependency is missing a cpakid.");
        else if (!node["cpakid"].IsScalar())
            throw YAML::Exception(node.Mark(), "Dependency cpakid must be a string.");

        // Validate optional fields.
        if (node["repository"] && !node["repository"].IsMap())
            throw YAML::Exception(node.Mark(), "Dependency repository must be a map.");
    } else if (!node.IsScalar()) {
        throw YAML::Exception(node.Mark(), "Dependency must be a string or a map.");
    }
}


}


template<>
struct YAML::convert<cpak::OptionalProperty<cpak::Repository>> {
    static Node encode(const cpak::OptionalProperty<cpak::Repository>& rhs) {
        Node node;
        if (rhs != std::nullopt)
            node = *rhs;
        return node;
    }

    static bool decode(const Node& node, cpak::OptionalProperty<cpak::Repository>& rhs) {
        cpak::validateRepositorySchema(node);
        rhs = node.as<cpak::Repository>();
        return true;
    }
};


template<>
struct YAML::convert<cpak::Dependency> {
    static Node encode(const cpak::Dependency& rhs) {
        Node node;
        if (rhs.remote != std::nullopt) {
            node["cpakid"] = cpak::identityToString(rhs);
            node["remote"] = rhs.remote;
            return node;
        }

        node = cpak::identityToString(rhs);
        return node;
    }

    static bool decode(const Node& node, cpak::Dependency& rhs) {
        cpak::validateDependencySchema(node);
        if (node.IsScalar()) {
            const auto& identity =
                cpak::identityFromString(node.as<std::string>());
            
            rhs.name = identity.name;
            rhs.gpid = identity.gpid;
            rhs.semv = identity.semv;
            return true;
        }

        // Decode mapped dependency.
        const auto& identity =
            cpak::identityFromString(node["cpakid"].as<std::string>());
        
        rhs.name = identity.name;
        rhs.gpid = identity.gpid;
        rhs.semv = identity.semv;
        if (node["remote"])
            rhs.remote = node["remote"].as<cpak::Repository>();
        return true;
    }
};