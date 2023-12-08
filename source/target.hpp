#pragma once
#include "option.hpp"
#include "utilities/stropts.hpp"
#include <iomanip>
#include <optional>

namespace cpak {


struct BaseTarget {};


/// @brief Describes the different types of targets.
enum struct TargetType {
    Undefined  = -1,
    Executable =  0,
    Archive    =  1,
    Dynlib     =  2,

    // Alternative names.
    Sample         = Executable,
    Test           = Executable,
    StaticLibrary  = Archive,
    DynamicLibrary = Dynlib,
};


enum struct AccessLevel {
    ePublic,
    eProtected,
    ePrivate
};


template<typename TEvaluated>
struct TargetProperty {
    TEvaluated  value;
    std::string scalar;

    BaseTarget* owner{ nullptr };
    AccessLevel level{ AccessLevel::ePublic };

    TargetProperty() = default;
    TargetProperty(const std::string& unevaluated,
                   const TEvaluated&  defaultValue = {}) noexcept
        : value{ defaultValue }
        , scalar{ unevaluated }
    { }

    TargetProperty(const std::string& unevaluated,
                   BaseTarget* owner,
                   AccessLevel level = AccessLevel::ePublic) noexcept
        : scalar{ unevaluated }
        , owner{ owner }
        , level{ level }
    { }

    bool isPublic()    const noexcept { return level == AccessLevel::ePublic;    }
    bool isProtected() const noexcept { return level == AccessLevel::eProtected; }
    bool isPrivate()   const noexcept { return level == AccessLevel::ePrivate;   }
};


template<typename TEvaluated>
using PropertyList = std::vector<TargetProperty<TEvaluated>>;


/// @brief   Contains the search paths for a given target.
/// @details This is composed of include, system, and library paths. These
///          represent the paths that the compiler will search for headers,
///          system headers, and libraries respectively.
/// @remarks This is based on the options that are available in the GNU
///          toolchain.
struct SearchPaths {
    PropertyList<std::string> include;
    PropertyList<std::string> system;
    PropertyList<std::string> library;
};


struct CompilationProperties {
    virtual ~CompilationProperties() = default;

    PropertyList<std::string>  defines;
    PropertyList<std::string>  inherits;
    PropertyList<std::string>  libraries;
    PropertyList<std::string>  options;
    std::optional<SearchPaths> search;

    TargetProperty<bool> enabled{ "true", true };
};


struct DescriptiveTarget : public BaseTarget {
    virtual ~DescriptiveTarget() = default;

    TargetProperty<std::string> name{ "<INVALID>", };
    TargetProperty<std::string> desc{ "<INVALID>", };

    virtual std::string describe() const noexcept = 0;
};


// Used for tests and samples.
struct GenericTarget final
    : public DescriptiveTarget
    , public CompilationProperties
{
    GenericTarget() = default;
    ~GenericTarget() = default;

    TargetProperty<std::string> main;

    std::string describe() const noexcept { return ""; }
};


struct GenericBuildSection final
    : public BaseTarget
    , public CompilationProperties
{
    GenericBuildSection() = default;
    ~GenericBuildSection() = default;

    std::vector<GenericTarget> targets;
};


struct InterfaceTarget final
    : public DescriptiveTarget
    , public CompilationProperties
{
    InterfaceTarget() = default;
    ~InterfaceTarget() = default;

    PropertyList<std::string> sources;

    std::string describe() const noexcept { return ""; }
};


/// @brief   Contains the build target information.
/// @details ...
struct BuildTarget final
    : public DescriptiveTarget
    , public CompilationProperties
{
    BuildTarget() = default;
    ~BuildTarget() = default;

    PropertyList<std::string>  sources;
    TargetProperty<TargetType> type{ "Undefined", TargetType::Undefined };


    operator InterfaceTarget() const noexcept {
        InterfaceTarget target;
        target.name = this->name;
        target.desc = this->desc;
        target.defines = this->defines;
        target.inherits = this->inherits;
        target.libraries = this->libraries;
        target.options = this->options;
        target.search = this->search;
        target.sources = this->sources;
        return target;
    }


    template<typename TEvaluated>
    bool owns(const TargetProperty<TEvaluated>& property) {
        return property.owner == this;
    }

    std::string describe() const noexcept { return ""; }
};


/// @brief   Contains the install target information.
/// @details ...
struct InstallTarget {
    std::string source;
    std::string destination;
};


/// @brief  Builds a string representation of the target type.
/// @param  type The target type to build a string representation of.
/// @return The string that represents the target type.
inline std::string
buildTypeName(TargetType type) {
    switch (type) {
    case TargetType::Undefined:  return "Undefined";
    case TargetType::Executable: return "executable";
    case TargetType::Archive:    return "archive";
    case TargetType::Dynlib:     return "dynlib";
    }

    return "INVALID";
}


/// @brief  Builds a target type from a string representation.
/// @param  name The name of the target type to build.
/// @return The target type that was built.
inline TargetType
buildTypeFromName(std::string_view name) {
    if (name == "executable" || name == "sample" || name == "test")
        return cpak::TargetType::Executable;

    // TODO: phase out 'static library' and 'dynamic library'
    if (name == "static library" || name == "archive")
        return cpak::TargetType::StaticLibrary;
    
    if (name == "dynamic library" || name == "dynlib")
        return cpak::TargetType::DynamicLibrary;

    return cpak::TargetType::Undefined;
}



inline void
interpolateOptions(TargetType& outEvaluated,
                   const std::string& unevaluated,
                   const std::vector<BuildOption>& options) {
    auto toEvaluate = unevaluated;
    auto position   = std::string::size_type{ 0 };
    while ((position = toEvaluate.rfind("${", 0)) != std::string::npos)
        interpolateOptions(toEvaluate, toEvaluate, options);

    outEvaluated = buildTypeFromName(toEvaluate);
}


inline std::string
accessLevelToString(AccessLevel level) noexcept {
    switch (level) {
    case AccessLevel::ePublic:    return "!public";
    case AccessLevel::eProtected: return "!protected";
    case AccessLevel::ePrivate:   return "!private";
    }

    return "!";
}


inline AccessLevel
stringToAccessLevel(std::string_view str) noexcept {
    if (str == "!protected") return AccessLevel::eProtected;
    if (str == "!private")   return AccessLevel::ePrivate;

    return AccessLevel::ePublic;
}


/// @brief Validates the schema of the target type.
/// @param node The node containing the target type.
inline void
validateTargetTypeSchema(const YAML::Node& node) {
    if (!node.IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a scalar.");
}


inline void
validateTargetPropertySchema(const YAML::Node& node) {
    if (!node.IsScalar())
        throw YAML::Exception(node.Mark(), "Target Property must be a scalar.");

    // Check if there is a tag.
    const auto& tag = node.Tag();
    if (tag.empty() || tag == "?" || tag == "!")
        return;

    // Check tag is one of the 3 allowed on properties.
    if (tag != "!public"    &&
        tag != "!protected" &&
        tag != "!private")
        // throw YAML::Exception(node.Mark(), "Unknown tag associated with scalar.");
        throw YAML::Exception(node.Mark(), tag);
}


/// @brief Validates the schema of the search paths.
/// @param node The node containing the search paths.
inline void
validateSearchPathsSchema(const YAML::Node& node) {
    if (!node.IsMap())
        throw YAML::Exception(node.Mark(), "Search paths must be a map.");

    if (node["include"] && !node["include"].IsSequence())
        throw YAML::Exception(node.Mark(), "Include paths must be a sequence.");
    if (node["system"] && !node["system"].IsSequence())
        throw YAML::Exception(node.Mark(), "System paths must be a sequence.");
    if (node["library"] && !node["library"].IsSequence())
        throw YAML::Exception(node.Mark(), "Library paths must be a sequence.");
}


inline void
validateCompilationPropertiesSchema(const YAML::Node& node) {
    // Validate optional fields if present.
    if (node["defines"] && !node["defines"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target defines must be a sequence.");

    if (node["inherits"] && !node["inherits"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target inherits must be a sequence.");

    if (node["libraries"] && !node["libraries"].IsSequence())
        throw YAML::Exception(node.Mark(),
                              "Target libraries must be a sequence.");

    if (node["options"] &&
       !node["options"].IsScalar() &&
       !node["options"].IsSequence())
        throw YAML::Exception(node.Mark(), "Target options must be a scalar or sequence.");

    if (node["search"] && !node["search"].IsMap())
        throw YAML::Exception(node.Mark(), "Target search paths must be a map.");

    if (node["enabled"] && !node["enabled"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target enabled must be a boolean.");
}


inline void
validateDescriptiveTargetSchema(const YAML::Node& node) {
    if (!node["name"])
        throw YAML::Exception(node.Mark(), "Target must contain a scalar name.");

    if (node["desc"] && !node["desc"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target description must be a scalar.");
}


inline void
validateGenericTargetSchema(const YAML::Node& node) {
    // Generic targets can be a single string.
    if (node.IsScalar())
        return;

    validateDescriptiveTargetSchema(node);
    validateCompilationPropertiesSchema(node);

    if (!node["main"])
        throw YAML::Exception(node.Mark(), "Target must contain a main file.");
    else if (!node["main"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target main must be a scalar.");
}


inline void
validateGenericBuildSectionSchema(const YAML::Node& node) {
    validateCompilationPropertiesSchema(node);

    if (!node["targets"])
        throw YAML::Exception(node.Mark(), "Generic build section must contain targets.");
    else if (!node["targets"].IsSequence())
        throw YAML::Exception(node.Mark(), "Generic build section targets must be a sequence.");
}


inline void
validateInterfaceTargetSchema(const YAML::Node& node) {
    validateCompilationPropertiesSchema(node);
    validateDescriptiveTargetSchema(node);

    if (node["sources"]) {
        if (!node["sources"].IsSequence())
            throw YAML::Exception(node.Mark(),
                                  "Interface sources must be a sequence.");
        else if (node["sources"].size() == 0)
            throw YAML::Exception(node.Mark(),
                                  "Interface sources must not be empty.");
    }
}


/// @brief Validates the schema of the build target.
/// @param node The node containing the build target.
inline void
validateBuildTargetSchema(const YAML::Node& node) {
    // Validate required fields.
    validateDescriptiveTargetSchema(node);
    validateCompilationPropertiesSchema(node);

    if (!node["type"])
        throw YAML::Exception(node.Mark(), "Target is missing a type.");
    else if (!node["type"].IsScalar())
        throw YAML::Exception(node.Mark(), "Target type must be a string.");

    if (node["sources"]) {
        if (!node["sources"].IsSequence())
            throw YAML::Exception(node.Mark(),
                                  "Target sources must be a sequence.");
        else if (node["sources"].size() == 0)
            throw YAML::Exception(node.Mark(),
                                  "Target sources must not be empty.");
    }
}


inline PropertyList<std::string>
propertiesFromString(AccessLevel level,
                     std::string_view value,
                     char delimiter = ' ') noexcept {
    // Split the string into vector of strings, then assign.
    PropertyList<std::string> values;
    std::istringstream iss(value.data());
    std::string token;
    while (std::getline(iss, token, delimiter))
        values.push_back(TargetProperty<std::string>(token, nullptr, level));

    return values;
}


inline std::string
propertiesToString(const PropertyList<std::string>& properties,
                   char delimiter   = ' ',
                   bool unevaluated = true) noexcept {
    std::ostringstream oss;
    for (const auto& property : properties)
        oss << (unevaluated ? property.scalar : property.value) << delimiter;

    return oss.str();
}


inline void
assignOwnerToProperties(PropertyList<std::string>& properties,
                        BaseTarget* owner) noexcept {
    for (auto& property : properties)
        property.owner = owner;
}


/// @brief  Converts the given target to a string.
/// @param  target The target to convert to a string.
/// @return The string representation of the target.
inline std::string
to_string(const BuildTarget& target) noexcept {
    std::ostringstream oss;
    // oss << target.name << " (" << buildTypeName(target.type) << ") {\n";
    // oss << "    Defines: " << accessiblesToString(target.defines, ';')
    //     << "\n";
    // oss << "    Interfaces: " << accessiblesToString(target.interfaces, ';')
    //     << "\n";
    // oss << "    Libraries: " << accessiblesToString(target.libraries, ';')
    //     << "\n";
    // oss << "    Sources: " << accessiblesToString(target.sources, ';')
    //     << "\n";
    // oss << "    Options: "
    //     << accessiblesToString(target.options); // hasn't been trimmed yet.

    // if (target.search != std::nullopt) {
    //     oss << "    Search: {\n";
    //     oss << "        Include: "
    //         << accessiblesToString(target.search->include, ';') << "\n";
    //     oss << "        System: "
    //         << accessiblesToString(target.search->system, ';') << "\n";
    //     oss << "        Library: "
    //         << accessiblesToString(target.search->library, ';') << "\n";
    //     oss << "    }\n";
    // }

    // oss << "}";
    return oss.str();
}


inline std::string
describe(const BuildTarget& target) noexcept {
    // NOTE: All properties should display their evaluated values when being
    //       described to the user so they know what the loaded values are,
    //       not what the unevaluated expression is.
    // TODO: Maybe add a flag to support displaying unevaluated properties.
    std::ostringstream oss;
    oss << "\n" << target.name.value
        << " (" << buildTypeName(target.type.value)
        << ")\n\n";

    oss << "Enabled: "
        << std::boolalpha
        << target.enabled.value
        << "\n";
    
    if (!target.desc.value.empty())
        oss << "Description:\n  "
            << utilities::textWrap(target.desc.value, 50, "  ")
            << "\n";

    if (target.inherits.size() != 0) {
        oss << "Inherits from:\n";
        for (const auto& interface : target.inherits)
            oss << " - " << interface.value << "\n";
        oss << std::endl;
    }

    oss << "Compiled with:\n";
    if (target.search != std::nullopt) {
        for (const auto& sysInclude : target.search->system)
            oss << "-isystem " << sysInclude.value << "\n";

        for (const auto& regInclude : target.search->include)
            oss << "-I " << regInclude.value << "\n";

        for (const auto& libInclude : target.search->library)
            oss << "-L " << libInclude.value << "\n";
    }

    for (const auto& define : target.defines)
        oss << "-D " << define.value << "\n";

    if (target.options.size() != 0) {
        for (const auto& option : target.options)
            oss << option.value << " ";
        oss << "\n";
    }

    for (const auto& library : target.libraries)
        oss << "-l " << library.value << "\n";
    
    oss << "-c\n";
    for (const auto& source : target.sources)
        oss << "  " << source.value << "\n";

    return oss.str();
}


} // namespace cpak


template<>
struct YAML::convert<cpak::TargetType> {
    static Node
    encode(const cpak::TargetType& rhs) {
        Node node;
        node = cpak::buildTypeName(rhs);
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::TargetType& rhs) {
        cpak::validateTargetTypeSchema(node);
        rhs = cpak::buildTypeFromName(node.as<std::string>());
        return true;
    }
};


template<>
struct YAML::convert<cpak::TargetProperty<bool>> {
    static Node
    encode(const cpak::TargetProperty<bool>& rhs) {
        Node node;
        node = rhs.scalar;
        node.SetTag(cpak::accessLevelToString(rhs.level));
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::TargetProperty<bool>& rhs) {
        cpak::validateTargetPropertySchema(node);
        rhs.scalar = node.as<std::string>();
        rhs.level  = cpak::stringToAccessLevel(node.Tag());
        return true;
    }
};


template<>
struct YAML::convert<cpak::TargetProperty<cpak::TargetType>> {
    static Node
    encode(const cpak::TargetProperty<cpak::TargetType>& rhs) {
        Node node;
        node = cpak::buildTypeName(rhs.value);
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::TargetProperty<cpak::TargetType>& rhs) {
        cpak::validateTargetTypeSchema(node);
        rhs.scalar = node.as<std::string>();
        rhs.level  = cpak::stringToAccessLevel(node.Tag());
        return true;
    }
};


template<>
struct YAML::convert<cpak::TargetProperty<std::string>> {
    static Node
    encode(const cpak::TargetProperty<std::string>& rhs) {
        Node node;
        node = rhs.scalar;
        node.SetTag(cpak::accessLevelToString(rhs.level));
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::TargetProperty<std::string>& rhs) {
        cpak::validateTargetPropertySchema(node);
        
        // HACK: This assumes that the option interpolation will operate on the
        //       'value' which isn't necessarily correct scenario. This is not
        //       possible for the other kinds of target properties and should be
        //       amended when possible.
        rhs.value = rhs.scalar = node.as<std::string>();
        rhs.level = cpak::stringToAccessLevel(node.Tag());
        return true;
    }
};


template<>
struct YAML::convert<cpak::SearchPaths> {
    static Node
    encode(const cpak::SearchPaths& rhs) {
        Node node;
        node["include"] = rhs.include;
        node["system"]  = rhs.system;
        node["library"] = rhs.library;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::SearchPaths& rhs) {
        cpak::validateSearchPathsSchema(node);
        cpak::SearchPaths paths;
        if (node["include"]) paths.include = node["include"].as<cpak::PropertyList<std::string>>();
        if (node["system"])  paths.system  = node["system"].as<cpak::PropertyList<std::string>>();
        if (node["library"]) paths.library = node["library"].as<cpak::PropertyList<std::string>>();
        
        rhs = paths;
        return true;
    }
};

template<>
struct YAML::convert<cpak::GenericTarget> {
    static Node
    encode(const cpak::GenericTarget& rhs) {
        using namespace cpak;

        Node node;
        if (rhs.search == std::nullopt  &&
            rhs.enabled.value           &&
            rhs.desc.value.empty()      &&
            rhs.options.empty()         &&
            rhs.defines.empty()         &&
            rhs.libraries.empty()       &&
            rhs.inherits.empty())
        {
            // If there is no other information about this generic target,
            // output short form.
            node = rhs.main;
            return node;
        }

        node["name"] = rhs.name;
        node["main"] = rhs.main;
        if (!rhs.desc.value.empty())
            node["desc"] = rhs.desc.value;

        node["enabled"] = rhs.enabled;
        
        // Handle optional fields.
        if (rhs.search != std::nullopt)
            node["search"] = rhs.search.value();
        
        if (!rhs.options.empty()) {
            bool isAllSameAccess;
            auto accessLevel = rhs.options[0].level;
            for (const auto& option : rhs.options) {
                if (option.level != accessLevel) {
                    isAllSameAccess = false;
                    break;
                }
            }

            if (isAllSameAccess) {
                const auto tagName = accessLevelToString(accessLevel);
                node["options"] = propertiesToString(rhs.options);
                node.SetTag(tagName);
            } else node["options"] = rhs.options;
        }

        if (!rhs.defines.empty())   node["defines"]   = rhs.defines;
        if (!rhs.libraries.empty()) node["libraries"] = rhs.libraries;
        if (!rhs.inherits.empty())  node["inherits"]  = rhs.inherits;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::GenericTarget& rhs) {
        using namespace cpak;

        validateGenericTargetSchema(node);
        if (node.IsScalar()) {
            rhs.main = node.as<TargetProperty<std::string>>();
            rhs.enabled = TargetProperty<bool> { "true", true };
            return true;
        }

        rhs.name = node["name"].as<std::string>();
        if (node["desc"])
            rhs.desc = node["desc"].as<std::string>();
        
        if (node["enabled"]) {
            rhs.enabled = node["enabled"].as<TargetProperty<bool>>();
        } else {
            rhs.enabled = TargetProperty<bool>{ "true", true };
        }

        // Check for sources if this isn't an interface target.
        auto* target = &rhs;

        // Handle optional fields.
        if (node["search"]) {
            rhs.search = node["search"].as<SearchPaths>();
            assignOwnerToProperties(rhs.search->include, target);
            assignOwnerToProperties(rhs.search->system, target);
            assignOwnerToProperties(rhs.search->library, target);
        }

        if (node["options"]) {
            if (node["options"].IsScalar()) {
                const auto tag = node["options"].Tag();
                const auto level = stringToAccessLevel(tag);
                const auto& opts = cpak::utilities::trim(node["options"].as<std::string>());
                rhs.options = propertiesFromString(level, opts);
                assignOwnerToProperties(rhs.options, target);
            } else if (node["options"].IsSequence()) {
                rhs.options = node["options"].as<cpak::PropertyList<std::string>>();
                assignOwnerToProperties(rhs.options, target);
            }
        }

        if (node["defines"]) {
            rhs.defines = node["defines"].as<PropertyList<std::string>>();
            assignOwnerToProperties(rhs.defines, target);
        }

        if (node["libraries"]) {
            rhs.libraries = node["libraries"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.libraries, target);
        }

        if (node["inherits"]) {
            rhs.inherits = node["inherits"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.inherits, target);
        }

        return true;
    }
};


template<>
struct YAML::convert<cpak::GenericBuildSection> {
    static Node
    encode(const cpak::GenericBuildSection& rhs) {
        using namespace cpak;

        Node node;
        node["enabled"] = rhs.enabled;
        
        // Handle optional fields.
        if (rhs.search != std::nullopt)
            node["search"] = rhs.search.value();
        
        if (!rhs.options.empty()) {
            bool isAllSameAccess;
            auto accessLevel = rhs.options[0].level;
            for (const auto& option : rhs.options) {
                if (option.level != accessLevel) {
                    isAllSameAccess = false;
                    break;
                }
            }

            if (isAllSameAccess) {
                const auto tagName = accessLevelToString(accessLevel);
                node["options"] = propertiesToString(rhs.options);
                node.SetTag(tagName);
            } else node["options"] = rhs.options;
        }

        if (!rhs.defines.empty())   node["defines"]   = rhs.defines;
        if (!rhs.libraries.empty()) node["libraries"] = rhs.libraries;
        if (!rhs.inherits.empty())  node["inherits"]  = rhs.inherits;

        node["targets"] = rhs.targets;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::GenericBuildSection& rhs) {
        using namespace cpak;

        validateGenericBuildSectionSchema(node);
        if (node["enabled"]) {
            rhs.enabled = node["enabled"].as<TargetProperty<bool>>();
        } else {
            rhs.enabled = TargetProperty<bool>{ "true", true };
        }

        // Check for sources if this isn't an interface target.
        auto* target = &rhs;

        // Handle optional fields.
        if (node["search"]) {
            rhs.search = node["search"].as<SearchPaths>();
            assignOwnerToProperties(rhs.search->include, target);
            assignOwnerToProperties(rhs.search->system, target);
            assignOwnerToProperties(rhs.search->library, target);
        }

        if (node["options"]) {
            if (node["options"].IsScalar()) {
                const auto tag = node["options"].Tag();
                const auto level = stringToAccessLevel(tag);
                const auto& opts = cpak::utilities::trim(node["options"].as<std::string>());
                rhs.options = propertiesFromString(level, opts);
                assignOwnerToProperties(rhs.options, target);
            } else if (node["options"].IsSequence()) {
                rhs.options = node["options"].as<cpak::PropertyList<std::string>>();
                assignOwnerToProperties(rhs.options, target);
            }
        }

        if (node["defines"]) {
            rhs.defines = node["defines"].as<PropertyList<std::string>>();
            assignOwnerToProperties(rhs.defines, target);
        }

        if (node["libraries"]) {
            rhs.libraries = node["libraries"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.libraries, target);
        }

        if (node["inherits"]) {
            rhs.inherits = node["inherits"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.inherits, target);
        }

        rhs.targets = node["targets"].as<std::vector<cpak::GenericTarget>>();
        return true;
    }
};


template<>
struct YAML::convert<cpak::InterfaceTarget> {
    static Node
    encode(const cpak::InterfaceTarget& rhs) {
        using namespace cpak;

        Node node;
        node["name"] = rhs.name;
        if (!rhs.desc.value.empty())
            node["desc"] = rhs.desc.value;

        node["enabled"] = rhs.enabled;

        if (node["sources"])
            node["sources"] = rhs.sources;
    
        // Handle optional fields.
        if (rhs.search != std::nullopt)
            node["search"] = rhs.search.value();
        
        if (!rhs.options.empty()) {
            bool isAllSameAccess;
            auto accessLevel = rhs.options[0].level;
            for (const auto& option : rhs.options) {
                if (option.level != accessLevel) {
                    isAllSameAccess = false;
                    break;
                }
            }

            if (isAllSameAccess) {
                const auto tagName = accessLevelToString(accessLevel);
                node["options"] = propertiesToString(rhs.options);
                node.SetTag(tagName);
            } else node["options"] = rhs.options;
        }

        if (!rhs.defines.empty())   node["defines"]   = rhs.defines;
        if (!rhs.libraries.empty()) node["libraries"] = rhs.libraries;
        if (!rhs.inherits.empty())  node["inherits"]  = rhs.inherits;
        return node;
    }

    static bool
    decode(const YAML::Node& node, cpak::InterfaceTarget& rhs) {
        using namespace cpak;

        validateInterfaceTargetSchema(node);
        rhs.name = node["name"].as<std::string>();
        if (node["desc"])
            rhs.desc = node["desc"].as<std::string>();
        
        if (node["enabled"]) {
            rhs.enabled = node["enabled"].as<TargetProperty<bool>>();
        } else {
            rhs.enabled = TargetProperty<bool>{ "true", true };
        }

        // Check for sources if this isn't an interface target.
        auto* target = &rhs;
        if (node["sources"]) {
            rhs.sources = node["sources"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.sources, target);
        }

        // Handle optional fields.
        if (node["search"]) {
            rhs.search = node["search"].as<SearchPaths>();
            assignOwnerToProperties(rhs.search->include, target);
            assignOwnerToProperties(rhs.search->system, target);
            assignOwnerToProperties(rhs.search->library, target);
        }

        if (node["options"]) {
            if (node["options"].IsScalar()) {
                const auto tag = node["options"].Tag();
                const auto level = stringToAccessLevel(tag);
                const auto& opts = cpak::utilities::trim(node["options"].as<std::string>());
                rhs.options = propertiesFromString(level, opts);
                assignOwnerToProperties(rhs.options, target);
            } else if (node["options"].IsSequence()) {
                rhs.options = node["options"].as<cpak::PropertyList<std::string>>();
                assignOwnerToProperties(rhs.options, target);
            }
        }

        if (node["defines"]) {
            rhs.defines = node["defines"].as<PropertyList<std::string>>();
            assignOwnerToProperties(rhs.defines, target);
        }

        if (node["libraries"]) {
            rhs.libraries = node["libraries"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.libraries, target);
        }

        if (node["inherits"]) {
            rhs.inherits = node["inherits"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.inherits, target);
        }

        return true;
    }
};


template<>
struct YAML::convert<cpak::BuildTarget> {
    static Node
    encode(const cpak::BuildTarget& rhs) {
        using namespace cpak;

        Node node;
        node["type"] = rhs.type;
        node["name"] = rhs.name;
        if (!rhs.desc.value.empty())
            node["desc"] = rhs.desc.value;

        node["enabled"] = rhs.enabled;
        node["sources"] = rhs.sources;
    
        // Handle optional fields.
        if (rhs.search != std::nullopt)
            node["search"] = rhs.search.value();
        
        if (!rhs.options.empty()) {
            bool isAllSameAccess;
            auto accessLevel = rhs.options[0].level;
            for (const auto& option : rhs.options) {
                if (option.level != accessLevel) {
                    isAllSameAccess = false;
                    break;
                }
            }

            if (isAllSameAccess) {
                const auto tagName = accessLevelToString(accessLevel);
                node["options"] = propertiesToString(rhs.options);
                node.SetTag(tagName);
            } else node["options"] = rhs.options;
        }

        if (!rhs.defines.empty())   node["defines"]   = rhs.defines;
        if (!rhs.libraries.empty()) node["libraries"] = rhs.libraries;
        if (!rhs.inherits.empty())  node["inherits"]  = rhs.inherits;
        return node;
    }

    static bool
    decode(const Node& node, cpak::BuildTarget& rhs) {
        using namespace cpak;

        validateBuildTargetSchema(node);
        rhs.name = node["name"].as<std::string>();
        rhs.type = node["type"].as<TargetProperty<TargetType>>();

        if (node["desc"])
            rhs.desc = node["desc"].as<std::string>();
        
        if (node["enabled"]) {
            rhs.enabled = node["enabled"].as<TargetProperty<bool>>();
        } else {
            rhs.enabled = TargetProperty<bool>{ "true", true };
        }

        // Check for sources if this isn't an interface target.
        auto* target = &rhs;
        if (!node["sources"])
            throw YAML::Exception(node.Mark(), "Target is missing sources.");
        rhs.sources = node["sources"].as<cpak::PropertyList<std::string>>();
        assignOwnerToProperties(rhs.sources, target);

        // Handle optional fields.
        if (node["search"]) {
            rhs.search = node["search"].as<SearchPaths>();
            assignOwnerToProperties(rhs.search->include, target);
            assignOwnerToProperties(rhs.search->system, target);
            assignOwnerToProperties(rhs.search->library, target);
        }

        if (node["options"]) {
            if (node["options"].IsScalar()) {
                const auto tag = node["options"].Tag();
                const auto level = stringToAccessLevel(tag);
                const auto& opts = cpak::utilities::trim(node["options"].as<std::string>());
                rhs.options = propertiesFromString(level, opts);
                assignOwnerToProperties(rhs.options, target);
            } else if (node["options"].IsSequence()) {
                rhs.options = node["options"].as<cpak::PropertyList<std::string>>();
                assignOwnerToProperties(rhs.options, target);
            }
        }

        if (node["defines"]) {
            rhs.defines = node["defines"].as<PropertyList<std::string>>();
            assignOwnerToProperties(rhs.defines, target);
        }

        if (node["libraries"]) {
            rhs.libraries = node["libraries"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.libraries, target);
        }

        if (node["inherits"]) {
            rhs.inherits = node["inherits"].as<cpak::PropertyList<std::string>>();
            assignOwnerToProperties(rhs.inherits, target);
        }

        return true;
    }
};