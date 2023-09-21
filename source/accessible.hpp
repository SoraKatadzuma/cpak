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
class Accessible {
public:
    Accessible() noexcept = default;
    Accessible(const TStored& stored, AccessLevel level) noexcept
        : stored_(stored), level_(level) {}
    
    Accessible(TStored&& stored, AccessLevel level) noexcept
        : stored_(std::move(stored)), level_(level) {}

    Accessible(const Accessible& other) noexcept
        : stored_(other.stored_), level_(other.level_) {}
    
    Accessible(Accessible&& other) noexcept
        : stored_(std::move(other.stored_)), level_(other.level_) {}

    Accessible& operator=(const Accessible& other) noexcept {
        stored_ = other.stored_;
        level_  = other.level_;
        return *this;
    }

    Accessible& operator=(Accessible&& other) noexcept {
        stored_ = std::move(other.stored_);
        level_  = other.level_;
        return *this;
    }

    const TStored& stored() const noexcept {
        return stored_;
    }

    TStored& stored() noexcept {
        return stored_;
    }

    AccessLevel level() const noexcept {
        return level_;
    }

    bool isPublic() const noexcept {
        return level_ == AccessLevel::ePublic;
    }

    bool isProtected() const noexcept {
        return level_ == AccessLevel::eProtected;
    }

    bool isPrivate() const noexcept {
        return level_ == AccessLevel::ePrivate;
    }

    const BuildTarget* owner() const noexcept {
        return owner_;
    }

    BuildTarget* owner() noexcept {
        return owner_;
    }

    void setOwner(BuildTarget* target) noexcept {
        owner_ = target;
    }

    bool operator==(const Accessible& other) const noexcept {
        return stored_ == other.stored_ &&
               level_  == other.level_;
    }

    bool operator!=(const Accessible& other) const noexcept {
        return !(*this == other);
    }

private:
    TStored      stored_;
    BuildTarget* owner_ = nullptr;
    AccessLevel  level_ = AccessLevel::ePublic;
};


template<typename TStored>
using Accessibles = std::vector<Accessible<TStored>>;

}


template<>
struct YAML::convert<cpak::Accessible<std::string>> {
    static Node
    encode(const cpak::Accessible<std::string>& rhs) {
        Node node;
        switch (rhs.level()) {
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

        node = rhs.stored();
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::Accessible<std::string>& rhs) {
        if (node.Tag() == "!public")
            rhs = cpak::Accessible<std::string>(node.as<std::string>(),
                                                cpak::AccessLevel::ePublic);
        else if (node.Tag() == "!protected")
            rhs = cpak::Accessible<std::string>(node.as<std::string>(),
                                                cpak::AccessLevel::eProtected);
        else if (node.Tag() == "!private")
            rhs = cpak::Accessible<std::string>(node.as<std::string>(),
                                                cpak::AccessLevel::ePrivate);
        else
            rhs = cpak::Accessible<std::string>(node.as<std::string>(),
                                                cpak::AccessLevel::ePublic);
        
        return true;
    }
};