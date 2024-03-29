#pragma once
#include "version.hpp"

namespace cpak {


/// @brief   Contains the components of a CPakID.
/// @details A CPakID is a unique identifier for a CPak package. These are used
///          to identify packages and reference their versions.
struct Identity {
    std::string name;
    std::string gpid;
    cpak::version semv;

    bool isMapped{ false };
    bool versionIsBranch{ false };
};


inline void
validateIdentitySchema(const YAML::Node& node) {
    // Get identity as mapping or scalar.
    if (!node.IsMap() && !node.IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Identity must be a string or a map.");

    // If the identity is a mapping, validate the fields.
    if (node.IsMap()) {
        // Validate required fields.
        if (!node["name"])
            throw YAML::Exception(node.Mark(), "Identity is missing a name.");
        else if (!node["name"].IsScalar())
            throw YAML::Exception(node.Mark(),
                                  "Identity name must be a string.");

        if (!node["gpid"])
            throw YAML::Exception(node.Mark(), "Identity is missing a gpid.");
        else if (!node["gpid"].IsScalar())
            throw YAML::Exception(node.Mark(),
                                  "Identity gpid must be a string.");

        if (!node["semv"])
            throw YAML::Exception(node.Mark(), "Identity is missing a semv.");
        else if (!node["semv"].IsScalar())
            throw YAML::Exception(node.Mark(),
                                  "Identity semv must be a string.");
    }


    constexpr std::string_view kIdentityRegex =
        "^[a-z\\d](?:[a-z\\d]|-(?=[a-z\\d])){0,38}/"
        "[a-z\\d](?:[a-z\\d]|-(?=[a-z\\d])){0,38}@"
        "(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)"
        "(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|"
        "\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"
        "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

    static std::regex identityRegex(kIdentityRegex.data());

    // If the identity is a scalar, check it with a regex.
    // We expect the form to be gpid/name@semv.
    std::cmatch match;
    if (!std::regex_match(node.as<std::string>().c_str(), match, identityRegex))
        throw YAML::Exception(node.Mark(), "Identity is not a valid CPakID.");
}


/// @brief  Breaks down the given CPakID into its components.
/// @param  cpakid The CPakID to break down.
/// @return An Identity struct containing the CPakID components.
inline Identity
identityFromString(std::string_view cpakid, bool useBranch = false) {
    Identity result;
    std::string token;
    std::istringstream iss(cpakid.data());

    // Split the CPakID into its components.
    // Format: gpid/name@semv
    if (cpakid.find_first_of('/') == std::string::npos)
        throw std::runtime_error(
            "Invalid CPakID format, missing group id or name.");
    std::getline(iss, token, '/');
    result.gpid = token;

    if (cpakid.find_first_of('@') == std::string::npos)
        throw std::runtime_error(
            "Invalid CPakID format, missing name or version.");
    std::getline(iss, token, '@');
    result.name = token;

    std::getline(iss, token);
    if (token.empty())
        throw std::runtime_error("Invalid CPakID format, missing version.");
    
    const auto versionString = useBranch
        ? "0.0.0-" + token
        : token;

    result.semv = version::parse(versionString);
    result.versionIsBranch = useBranch;
    return result;
}


/// @brief  Converts the given Identity into a string.
/// @param  identity The Identity to convert.
/// @return The Identity as a string.
inline std::string
identityToString(const Identity& identity) {
    std::ostringstream oss;
    oss << identity.gpid << '/' << identity.name << '@' << identity.semv;
    return oss.str();
}


} // namespace cpak


template<>
struct YAML::convert<cpak::Identity> {
    static Node
    encode(const cpak::Identity& rhs) {
        Node node;
        if (rhs.isMapped) {
            node["name"] = rhs.name;
            node["gpid"] = rhs.gpid;
            node["semv"] = rhs.semv;
            return node;
        }

        node = cpak::identityToString(rhs);
        return node;
    }

    static bool
    decode(const Node& node, cpak::Identity& rhs) {
        cpak::validateIdentitySchema(node);
        if (node.IsMap()) {
            rhs.name     = node["name"].as<std::string>();
            rhs.gpid     = node["gpid"].as<std::string>();
            rhs.semv     = node["semv"].as<cpak::version>();
            rhs.isMapped = true;
            return true;
        }

        auto identity = cpak::identityFromString(node.as<std::string>());
        rhs.name      = identity.name;
        rhs.gpid      = identity.gpid;
        rhs.semv      = identity.semv;
        rhs.isMapped  = false;
        return true;
    }
};