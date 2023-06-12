#pragma once
#include "common.hpp"


namespace cpak {

// We use this internally.
using namespace semver;

} // namespace cpak


template<>
struct YAML::convert<cpak::version> {
    static Node
    encode(const cpak::version& rhs) {
        return Node(rhs.str());
    }

    static bool
    decode(const Node& node, cpak::version& rhs) {
        try {
            rhs = cpak::version::parse(node.as<std::string>());
        } catch (const std::runtime_error& exc) {
            // Rethrow the exception with the YAML node's mark.
            throw YAML::Exception(node.Mark(), exc.what());
        }

        return true;
    }
};