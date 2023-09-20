#pragma once


namespace cpak {


/// @brief   Contains information for pulling from a repository.
/// @details Because not all projects will be hosted on GitHub, we need to
///          support other repositories. This struct contains the information
///          needed to pull from those other repositories.
struct Repository {
    std::string address;
    std::string username;
    std::string email;
    std::string password;
};


/// @brief Validates the given repository schema.
/// @param node The node containing the repository.
inline void
validateRepositorySchema(const YAML::Node& node) {
    // Validate required fields.
    if (!node["address"])
        throw YAML::Exception(node.Mark(), "Repository is missing an address.");
    else if (!node["address"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Repository address must be a string.");

    // Validate optional fields.
    if (node["username"] && !node["username"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Repository username must be a string.");

    if (node["email"] && !node["email"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Repository email must be a string.");

    if (node["password"] && !node["password"].IsScalar())
        throw YAML::Exception(node.Mark(),
                              "Repository password must be a string.");
}


} // namespace cpak


template<>
struct YAML::convert<cpak::Repository> {
    static Node
    encode(const cpak::Repository& rhs) {
        Node node;

        node["address"]  = rhs.address;
        node["username"] = rhs.username;
        node["email"]    = rhs.email;
        node["password"] = rhs.password;

        return node;
    }

    static bool
    decode(const Node& node, cpak::Repository& rhs) {
        cpak::validateRepositorySchema(node);

        rhs.address = node["address"].as<std::string>();
        if (node["username"]) rhs.username = node["username"].as<std::string>();
        if (node["email"]) rhs.email = node["email"].as<std::string>();
        if (node["password"]) rhs.password = node["password"].as<std::string>();
        return true;
    }
};