#pragma once
#include "common.hpp"

namespace cpak {


// Q: How can we activate a profile?
// A: We can activate a profile by:
//    - Allowing user to override the profile with a command line argument.
//    - Automatically switching the profile based on platform.
//    - Automatically switching the profile based on build type.


// Q: What all should a profile contain?
// A: A profile should contain:
//    - The name of the profile.
//    - Whether it's the default/primary profile.
//    - The compiler to use.
struct Profile {
    std::string name;


    bool primary = false;
};


inline void
validateProfileSchema(const YAML::Node& node) {

}


}


template<>
struct YAML::convert<cpak::Profile> {
    static Node
    encode(const cpak::Profile& rhs) {
        Node node;
        node["name"] = rhs.name;

        if (rhs.primary)
            node["primary"] = rhs.primary;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::Profile& rhs) {

        return true;
    }
};