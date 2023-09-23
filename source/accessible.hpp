#pragma once
#include "common.hpp"

namespace cpak {


class BuildTarget;

enum struct AccessLevel {
    ePublic,
    eProtected,
    ePrivate
};


template<typename TStored>
struct Accessible {
    TStored      stored;
    BuildTarget* owner = nullptr;
    AccessLevel  level = AccessLevel::ePublic;


    bool isPublic() const noexcept {
        return level == AccessLevel::ePublic;
    }

    bool isProtected() const noexcept {
        return level == AccessLevel::eProtected;
    }

    bool isPrivate() const noexcept {
        return level == AccessLevel::ePrivate;
    }
};


template<typename TStored>
using Accessibles = std::vector<Accessible<TStored>>;

}


template<>
struct YAML::convert<cpak::Accessible<std::string>> {
    static Node
    encode(const cpak::Accessible<std::string>& rhs) {
        Node node;
        switch (rhs.level) {
        case cpak::AccessLevel::ePublic:
            node.SetTag("public");
            break;
        case cpak::AccessLevel::eProtected:
            node.SetTag("protected");
            break;
        case cpak::AccessLevel::ePrivate:
            node.SetTag("private");
            break;
        }

        node = rhs.stored;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::Accessible<std::string>& rhs) {
        if (node.Tag() == "!protected")
            rhs = cpak::Accessible<std::string> {
                .stored = node.as<std::string>(),
                .level  = cpak::AccessLevel::eProtected
            };
        else if (node.Tag() == "!private")
            rhs = cpak::Accessible<std::string> {
                .stored = node.as<std::string>(),
                .level  = cpak::AccessLevel::ePrivate
            };
        else
            rhs = cpak::Accessible<std::string> {
                .stored = node.as<std::string>(),
                .level  = cpak::AccessLevel::ePublic
            };
        
        return true;
    }
};