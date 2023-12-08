#include "cpakfile.hpp"
#include "errorcode.hpp"
#include "pipeline.hpp"
#include "utilities/logging.hpp"
#include "glob/glob.h"


namespace fs = std::filesystem;

using namespace cpak;

using std::string;
using std::string_view;
using std::vector;

using BuildQueue = std::queue<std::function<std::error_code()>>;
using DependencyCache = std::unordered_map<std::string_view, const cpak::CPakFile*>;
using InterfaceCache = std::unordered_map<std::string_view, const cpak::InterfaceTarget*>;
using LibraryCache = std::unordered_map<std::string_view, const cpak::CPakFile*>;

// Referenced from application.cpp
// Will need to rework how things are shared between compilation units.
extern BuildQueue buildQueue;
extern DependencyCache dependencyCache;
extern InterfaceCache interfaceCache;
extern LibraryCache libraryCache;


void reserveAndAppendFormatted(vector<string>& into,
                               const PropertyList<string>& from,
                               const char* pattern = "{}") noexcept {
    if (from.empty()) return;

    // Reserve space incase we need it.
    into.reserve(into.size() + from.size());
    for (const auto& value : from)
        into.emplace_back(fmt::format(fmt::runtime(pattern), value.value));
}


std::error_code
executeInShell(const vector<string>& arguments) {
    auto logger = spdlog::get("cpak");

    std::ostringstream oss;
    for (const auto& argument : arguments) oss << argument << " ";
    logger->debug(oss.str());

    auto command = subprocess::Popen(
        arguments, subprocess::output{ subprocess::PIPE },
        subprocess::error{ subprocess::PIPE }, subprocess::shell{ true });

    auto [output, error] = command.communicate();
    if (output.length > 0) logger->info("{}", output.buf.data());
    if (error.length > 0) logger->error("{}", error.buf.data());

    return make_error_code((errc::values)command.retcode());
}


void
copyIfAccessible(PropertyList<string>& lhs,
           const PropertyList<string>& rhs,
           const BuildTarget* target) noexcept {
    std::copy_if(
        rhs.begin(),
        rhs.end(),
        std::back_inserter(lhs),
        [=](const auto& value) {
            return value.isPublic() ||
                (value.isPrivate() && value.owner == target);
        });
}


void
copyInterfacePropertiesToTarget(const InterfaceTarget& interface,
                                      BuildTarget&     target) noexcept {
    auto tgtPtr = &target;
    target.name = interface.name;
    copyIfAccessible(target.defines, interface.defines, tgtPtr);
    copyIfAccessible(target.libraries, interface.libraries, tgtPtr);
    copyIfAccessible(target.sources, interface.sources, tgtPtr);
    copyIfAccessible(target.options, interface.options, tgtPtr);
    if (interface.search != std::nullopt) {
        if (target.search == std::nullopt) target.search = SearchPaths();
        copyIfAccessible(target.search->include, interface.search->include, tgtPtr);
        copyIfAccessible(target.search->system, interface.search->system, tgtPtr);
        copyIfAccessible(target.search->library, interface.search->library, tgtPtr);
    }
}


std::tuple<BuildTarget, std::error_code>
flattenInterfaceTarget(const vector<BuildTarget>& targets,
                       const InterfaceTarget& interface) noexcept {
    BuildTarget target;
    std::error_code status;
    for (const auto& inherited : interface.inherits) {
        if (!interfaceCache.contains(inherited.value)) {
            status = make_error_code(errc::interfaceNotFound);
            return std::make_tuple(target, status);
        }

        std::tie(target, status) =
            flattenInterfaceTarget(targets, *interfaceCache[inherited.value]);
    }

    copyInterfacePropertiesToTarget(interface, target);
    return std::make_tuple(target, status);
}


inline std::tuple<BuildTarget, std::error_code>
constructConsolidatedTarget(const vector<BuildTarget>& targets,
                            const InterfaceTarget& from) noexcept {
    return flattenInterfaceTarget(targets, from);
}


vector<string>
gatherCompilationArguments(const BuildTarget& target) noexcept {
    vector<string> arguments;

    auto size = target.defines.size() +
                target.libraries.size() +
                4; // g++, options, source, and output.

    if (target.search != std::nullopt)
        size += target.search->include.size() +
                target.search->system.size();

    arguments.reserve(size);
    arguments.emplace_back("g++");
    if (!target.options.empty()) {
        auto options = propertiesToString(target.options, ' ', false);
        options      = utilities::trim(std::move(options));
        arguments.emplace_back(std::move(options));
    }

    reserveAndAppendFormatted(arguments, target.defines, "-D {}");
    if (target.search != std::nullopt) {
        reserveAndAppendFormatted(arguments, target.search->include, "-I {}");
        reserveAndAppendFormatted(arguments, target.search->system, "-isystem {}");
    }

    reserveAndAppendFormatted(arguments, target.libraries, "-l {}");
    return arguments;
}


vector<string>
gatherLinkingArguments(const CPakFile& cpakfile,
                       const BuildTarget& target,
                       const vector<string>& objects) noexcept {
    vector<string> arguments;

    auto size = objects.size() +
                target.libraries.size() +
                (dependencyCache.size() * 2) +
                5; // g++, binaries, libaries, options, output.

    if (target.search != std::nullopt)
        size += target.search->library.size();

    arguments.reserve(size);
    arguments.emplace_back("g++");

    const auto binaryPath    = cpakfile.binaryBuildPath();
    const auto librariesPath = cpakfile.libraryBuildPath();
    for (auto& objectPath : objects) arguments.emplace_back(objectPath);

    // If we're linking a dependency, we'll automatically add it's binary and
    // library paths to the linking command.
    for (const auto& iter : dependencyCache) {
        const auto& dependency = iter.second;
        arguments.emplace_back(
            fmt::format("-L {}", dependency->binaryBuildPath().c_str()));
        arguments.emplace_back(
            fmt::format("-L {}", dependency->libraryBuildPath().c_str()));
    }

    if (target.search != std::nullopt)
        reserveAndAppendFormatted(arguments, target.search->library, "-L {}");

    reserveAndAppendFormatted(arguments, target.libraries, "-l {}");
    return arguments;
}


std::error_code
cpak::queueForBuild(const CPakFile& cpakfile,
                    const BuildTarget& target) noexcept {
    auto logger = spdlog::get("cpak");
    logger->info("Found target '{}'", target.name.value.c_str());
    auto [consolidated, result] =
        constructConsolidatedTarget(cpakfile.targets, target);

    // Interfaces don't share this anymore, so we need to copy it afterwards.
    consolidated.type = target.type;
    if (result.value() != errc::success)
        return result; // Let the caller handle the error.

    auto arguments = gatherCompilationArguments(consolidated);
    auto objects   = vector<string>();
    objects.reserve(target.sources.size());

    for (const auto& source : target.sources) {
        const auto sourcePath = cpakfile.projectPath / source.value;
        const auto objectPath =
            cpakfile.objectBuildPath() /
            fmt::format("{}.o", sourcePath.filename().c_str());

        logger->debug("Checking if source exists: {}", sourcePath.c_str());
        if (!fs::exists(sourcePath))
            return make_error_code(errc::pathDoesNotExist);

        // Add regardless of whether it exists or not, we're planning to build
        // it.
        logger->debug("Adding object: {}", objectPath.c_str());
        objects.emplace_back(objectPath.string());
        if (fs::exists(objectPath)) {
            const auto sourceTime = fs::last_write_time(sourcePath);
            const auto objectTime = fs::last_write_time(objectPath);
            if (sourceTime < objectTime) continue; // Does not need to be built.
        }

        // We can add to the copied vector, allowing us to still mutate it
        // outside of the lambda.
        buildQueue.push([=]() mutable -> std::error_code {
            arguments.emplace_back(fmt::format("-c {}", sourcePath.c_str()));
            arguments.emplace_back(fmt::format("-o {}", objectPath.c_str()));
            return executeInShell(arguments);
        });

        logger->debug("Queued for compilation: {}", sourcePath.string());
    }


    arguments = gatherLinkingArguments(cpakfile, consolidated, objects);
    buildQueue.push([=]() mutable -> std::error_code {
        string outputName;
        fs::path outputPath;

        logger->debug("target type: {}", buildTypeName(consolidated.type.value));

        switch (consolidated.type.value) {
        case TargetType::Executable:
            outputPath = cpakfile.binaryBuildPath() / consolidated.name.value;
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        case TargetType::StaticLibrary:
            outputName = fmt::format("lib{}.a", consolidated.name.value);
            outputPath = cpakfile.binaryBuildPath() / outputName;
            arguments.emplace_back("-r");
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        case TargetType::DynamicLibrary:
            outputName = fmt::format("lib{}.so", consolidated.name.value);
            outputPath = cpakfile.libraryBuildPath() / outputName;
            arguments.emplace_back("-shared");
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        default: assert(false && "Invalid target type");
        }

        return executeInShell(arguments);
    });

    logger->debug("Queued for Linking: {}", target.name.value.c_str());
    return result;
}


std::error_code
cpak::queueForBuild(const CPakFile& cpakfile) noexcept {
    auto logger      = spdlog::get("cpak");
    auto queueStatus = make_error_code(errc::success);
    logger->info("Building project: {}", cpakfile.projectPath.string());

    const auto binariesPath  = cpakfile.binaryBuildPath();
    const auto librariesPath = cpakfile.libraryBuildPath();
    const auto objectsPath   = cpakfile.objectBuildPath();
    if (!fs::exists(binariesPath)) fs::create_directories(binariesPath);
    if (!fs::exists(librariesPath)) fs::create_directories(librariesPath);
    if (!fs::exists(objectsPath)) fs::create_directories(objectsPath);

    for (const auto& dependency : cpakfile.dependencies) {
        const auto cpakid = identityToString(dependency);
        const auto iter   = dependencyCache.find(cpakid);
        if (iter == dependencyCache.end())
            return make_error_code(
                errc::failure); // Let the caller handle the error.

        queueStatus = cpak::queueForBuild(*iter->second);
        if (queueStatus.value() != errc::success)
            return queueStatus; // Let the caller handle the error.
    }

    for (const auto& target : cpakfile.targets) {
        if (!target.enabled.value) continue;

        queueStatus = cpak::queueForBuild(cpakfile, target);
        if (queueStatus.value() != errc::success)
            return queueStatus; // Let the caller handle the error.
    }

    return queueStatus;
}

std::error_code
cpak::executeBuild() noexcept {
    auto logger = spdlog::get("cpak");
    logger->info("Executing build queue with {} tasks", buildQueue.size());

    while (!buildQueue.empty()) {
        auto& task = buildQueue.front();

        const auto result = task();
        if (result.value() != errc::success)
            return result; // Let the caller handle the error.

        buildQueue.pop();
    }

    return make_error_code(errc::success);
}


void
installTarget(const BuildTarget& target,
              const CPakFile& cpakfile,
              const fs::path& binaryInstallPath,
              const fs::path& libraryInstallPath) noexcept {
    const auto targetName = target.name.value.c_str();
    const auto binaryBuildPath = cpakfile.binaryBuildPath();
    const auto libraryBuildPath = cpakfile.libraryBuildPath();
    
    auto logger = spdlog::get("cpak");
    logger->info("Installing target '{}'", targetName);
    if (target.type.value == TargetType::StaticLibrary) {
        const auto fileName = fmt::format("lib{}.a", targetName);
        const auto libPath = libraryBuildPath / fileName;
        const auto installPath = libraryInstallPath / fileName;
        fs::copy_file(libPath, installPath);
        logger->info("Installed archive '{}'", installPath.c_str());
    }

    if (target.type.value == TargetType::DynamicLibrary) {
    #if defined(_WIN32)
        const auto fileName = fmt::format("{}.dll", targetName);
    #else
        const auto fileName = fmt::format("lib{}.so", targetName);
    #endif

        const auto binPath = binaryBuildPath / fileName;
        const auto installPath = binaryInstallPath / fileName;
        fs::copy_file(binPath, installPath);
        logger->info("Installed dynlib '{}'", installPath.c_str());
    }

    if (target.type.value == TargetType::Executable) {
    #if defined(_WIN32)
        const auto fileName = fmt::format("{}.exe", targetName);
    #else
        const auto fileName = fmt::format("{}", targetName);
    #endif

        const auto binPath = binaryBuildPath / fileName;
        const auto installPath = binaryInstallPath / fileName;
        fs::copy_file(binPath, installPath);
        logger->info("Installed executable '{}'", installPath.c_str());
    }
}


std::error_code
installMultipleTargets(const CPakFile& cpakfile,
                       const fs::path& binaryInstallPath,
                       const fs::path& libraryInstallPath) noexcept {
    // Install the dependency itself.
    for (const auto& target : cpakfile.install->targets) {
        const auto iter = std::find_if(
            cpakfile.targets.begin(),
            cpakfile.targets.end(),
            [=](const auto& installTarget) {
                return target == installTarget.name.value;
            });

        if (iter == cpakfile.targets.end())
            return make_error_code(errc::installTargetNotFound);

        installTarget(*iter, cpakfile, binaryInstallPath, libraryInstallPath);
    }

    return make_error_code(errc::success);
}


void
installFiles(FileType type,
             const vector<fs::path>& files,
             const fs::path& binaryInstallPath,
             const fs::path& libraryInstallPath,
             const fs::path& includeInstallPath) noexcept {
    auto logger = spdlog::get("cpak");
    for (const auto& file : files) {
        switch (type) {
        case FileType::Header:
            fs::copy_file(file, includeInstallPath / file.filename());
            break;
        case FileType::Archive:
            fs::copy_file(file, libraryInstallPath / file.filename());
            break;
        case FileType::Dynlib:
        case FileType::Binary:
            fs::copy_file(file, binaryInstallPath / file.filename());
            break;
        }

        logger->info("Installed file '{}'", file.c_str());
    }
}


std::error_code
installMultipleFiles(const CPakFile& cpakfile,
                     const fs::path& binaryInstallPath,
                     const fs::path& libraryInstallPath,
                     const fs::path& includeInstallPath) noexcept {
    for (const auto& target : cpakfile.install->files) {
        installFiles(
            target.type,
            glob::rglob(target.glob),
            binaryInstallPath,
            libraryInstallPath,
            includeInstallPath);
    }

    return make_error_code(errc::success);
}


std::error_code
interanlInstallProject(const CPakFile& cpakfile,
                       const fs::path& binaryInstallPath,
                       const fs::path& libraryInstallPath,
                       const fs::path& includeInstallPath) noexcept {
    if (cpakfile.install == std::nullopt)
        return make_error_code(errc::noInstallInfo);

    auto logger = spdlog::get("cpak");
    logger->info("Installing Project '{}'", cpakfile.projectPath.c_str());

    // Install dependencies of the project.
    for (const auto& nestedDependency : cpakfile.dependencies) {
        const auto cpakid = identityToString(nestedDependency);
        const auto iter   = dependencyCache.find(cpakid);
        if (iter == dependencyCache.end())
            return make_error_code(errc::failure);

        interanlInstallProject(
            *iter->second,
            binaryInstallPath,
            libraryInstallPath,
            includeInstallPath);
    }

    // Install the project itself.
    auto result = make_error_code(errc::success);
    result = installMultipleTargets(
        cpakfile,
        binaryInstallPath,
        libraryInstallPath);
    if (result.value() != errc::success)
        return result;

    result = installMultipleFiles(
        cpakfile,
        binaryInstallPath,
        libraryInstallPath,
        includeInstallPath);
    if (result.value() != errc::success)
        return result;

    return result;
}


std::error_code
cpak::installProject(const CPakFile& cpakfile) noexcept {
    if (cpakfile.install == std::nullopt)
        return make_error_code(errc::noInstallInfo);

    const auto binaryInstallPath = cpakfile.binaryInstallPath();
    const auto libraryInstallPath = cpakfile.libraryInstallPath();
    const auto includeInstallPath = cpakfile.includeInstallPath();
    if (!fs::exists(binaryInstallPath)) fs::create_directories(binaryInstallPath);
    if (!fs::exists(libraryInstallPath)) fs::create_directories(libraryInstallPath);
    if (!fs::exists(includeInstallPath)) fs::create_directories(includeInstallPath);
    return interanlInstallProject(
        cpakfile,
        binaryInstallPath,
        libraryInstallPath,
        includeInstallPath);
}