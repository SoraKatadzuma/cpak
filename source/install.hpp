#pragma once
#include "common.hpp"

namespace cpak {


enum struct FileType {
    Header,
    Archive,
    Dynlib,
    Binary,
};


struct InstallFile {
    std::string glob;
    FileType    type;
};


struct Install {
    std::vector<std::string> targets;
    std::vector<InstallFile> files;
    bool global = false;
};


inline void
validateInstallSchema(const YAML::Node& node) {
    // When you specify an install in your CPakFile,
    // either, targets or files must be specified.

    if (!node["targets"] && !node["files"])
        throw YAML::ParserException(node.Mark(), "Either targets or files must be specified.");
    
    // If you specify targets, it must be a sequence of strings.
    if (!node["targets"].IsSequence())
        throw YAML::ParserException(node.Mark(), "Targets must be a sequence of strings.");
    
    // If you specify files, it must be a sequence of strings.
    if (!node["files"].IsSequence())
        throw YAML::ParserException(node.Mark(), "Files must be a sequence of strings.");

    // If you specify global, it must be a boolean.
    if (node["global"] && !node["global"].IsScalar())
        throw YAML::ParserException(node.Mark(), "Global must be a boolean.");
}


}


template<>
struct YAML::convert<cpak::InstallFile> {
    static Node
    encode(const cpak::InstallFile& rhs) {
        Node node;
        switch (rhs.type) {
        case cpak::FileType::Header:
            node.SetTag("header");
            break;
        case cpak::FileType::Archive:
            node.SetTag("archive");
            break;
        case cpak::FileType::Dynlib:
            node.SetTag("dynlib");
            break;
        case cpak::FileType::Binary:
            node.SetTag("binary");
            break;
        }

        node = rhs.glob;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::InstallFile& rhs) {
        if (!node.IsScalar())
            throw YAML::ParserException(node.Mark(), "Install file must be a string.");

        std::string_view type = node.Tag();
        if (type == "!header")
            rhs = cpak::InstallFile{ node.as<std::string>(), cpak::FileType::Header };
        else if (type == "!archive")
            rhs = cpak::InstallFile{ node.as<std::string>(), cpak::FileType::Archive };
        else if (type == "!dynlib")
            rhs = cpak::InstallFile{ node.as<std::string>(), cpak::FileType::Dynlib };
        else if (type == "!binary")
            rhs = cpak::InstallFile{ node.as<std::string>(), cpak::FileType::Binary };
        else
            throw YAML::ParserException(node.Mark(), "Unknown install file type.");

        return true;
    }
};


template<>
struct YAML::convert<cpak::Install> {
    static Node
    encode(const cpak::Install& rhs) {
        Node node;
        node["targets"] = rhs.targets;
        node["files"]   = rhs.files;
        node["global"]  = rhs.global;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::Install& rhs) {
        cpak::validateInstallSchema(node);
        if (node["targets"]) rhs.targets = node["targets"].as<std::vector<std::string>>();
        if (node["files"]) rhs.files = node["files"].as<std::vector<cpak::InstallFile>>();
        if (node["global"]) rhs.global = node["global"].as<bool>();
        return true;
    }
};