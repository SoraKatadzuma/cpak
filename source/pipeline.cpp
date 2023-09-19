#include "cpakfile.hpp"
#include "errorcode.hpp"
#include "pipeline.hpp"
#include "utilities/logging.hpp"


namespace fs = std::filesystem;


// Referenced from application.cpp
// Will need to rework how things are shared between compilation units.
extern std::queue<std::function<std::error_code()>> buildQueue;
extern std::unordered_map<std::string, cpak::CPakFile> dependencyCache;
extern std::unordered_map<std::string, const cpak::BuildTarget*> interfaceCache;


std::error_code
executeInShell(const std::vector<std::string>& arguments) {
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

    return cpak::make_error_code((cpak::errc::values)command.retcode());
}


void
copyInterfacePropertiesToTarget(const cpak::BuildTarget& interface,
                                cpak::BuildTarget& target) noexcept {
    target.name = interface.name;
    target.type = interface.type;
    target.defines |= interface.defines;
    target.libraries |= interface.libraries;
    target.sources |= interface.sources;
    target.options |= interface.options;
    if (interface.search != std::nullopt) {
        if (target.search == std::nullopt) target.search = cpak::SearchPaths();
        target.search->include |= interface.search->include;
        target.search->system |= interface.search->system;
        target.search->library |= interface.search->library;
    }
}


std::tuple<cpak::BuildTarget, std::error_code>
flattenInterfaceTarget(const std::vector<cpak::BuildTarget>& targets,
                       const cpak::BuildTarget& interface) noexcept {
    cpak::BuildTarget target;
    std::error_code status;
    for (const auto& inherited : *interface.interfaces) {
        if (!interfaceCache.contains(inherited)) {
            status = cpak::make_error_code(cpak::errc::interfaceNotFound);
            return std::make_tuple(target, status);
        }

        std::tie(target, status) =
            flattenInterfaceTarget(targets, *interfaceCache[inherited]);
    }

    copyInterfacePropertiesToTarget(interface, target);
    return std::make_tuple(target, status);
}


inline std::tuple<cpak::BuildTarget, std::error_code>
constructConsolidatedTarget(const std::vector<cpak::BuildTarget>& targets,
                            const cpak::BuildTarget& from) noexcept {
    return flattenInterfaceTarget(targets, from);
}


std::vector<std::string>
gatherCompilationArguments(const cpak::BuildTarget& target) noexcept {
    std::vector<std::string> arguments;

    auto size = target.defines->size() + target.libraries->size() +
                4; // g++, options, source, and output.

    if (target.search != std::nullopt)
        size += target.search->include->size() + target.search->system->size();

    arguments.reserve(size);
    arguments.emplace_back("g++");
    if (target.options != std::nullopt) {
        auto options = cpak::vectorPropertyToString(*target.options);
        options      = cpak::utilities::trim(std::move(options));
        arguments.emplace_back(std::move(options));
    }

    cpak::utilities::reserveAndAppendFormatted(arguments, *target.defines,
                                               "-D {}");
    if (target.search != std::nullopt) {
        cpak::utilities::reserveAndAppendFormatted(
            arguments, *target.search->include, "-I {}");
        cpak::utilities::reserveAndAppendFormatted(
            arguments, *target.search->system, "-isystem {}");
    }

    cpak::utilities::reserveAndAppendFormatted(arguments, *target.libraries,
                                               "-l {}");
    return arguments;
}


std::vector<std::string>
gatherLinkingArguments(const cpak::CPakFile& cpakfile,
                       const cpak::BuildTarget& target,
                       const std::vector<std::string>& objects) noexcept {
    std::vector<std::string> arguments;

    auto size = objects.size() + target.libraries->size() +
                (dependencyCache.size() * 2) +
                5; // g++, binaries, libaries, options, output.

    if (target.search != std::nullopt) size += target.search->library->size();

    arguments.reserve(size);
    arguments.emplace_back("g++");

    const auto binaryPath    = cpakfile.binariesBuildPath();
    const auto librariesPath = cpakfile.librariesBuildPath();
    for (auto& objectPath : objects) arguments.emplace_back(objectPath);

    // If we're linking a dependency, we'll automatically add it's binary and
    // library paths to the linking command.
    for (const auto& iter : dependencyCache) {
        const auto& dependency = iter.second;
        arguments.emplace_back(
            fmt::format("-L {}", dependency.binariesBuildPath().string()));
        arguments.emplace_back(
            fmt::format("-L {}", dependency.librariesBuildPath().string()));
    }

    if (target.search != std::nullopt)
        cpak::utilities::reserveAndAppendFormatted(
            arguments, *target.search->library, "-L {}");

    cpak::utilities::reserveAndAppendFormatted(arguments, *target.libraries,
                                               "-l {}");
    return arguments;
}


std::error_code
cpak::queueForBuild(const cpak::CPakFile& cpakfile,
                    const cpak::BuildTarget& target) noexcept {
    
    auto logger = spdlog::get("cpak");
    if (target.type == TargetType::Interface) {
        logger->info("Skipping interface target: {}", target.name->c_str());
        return make_error_code(errc::success);
    }

    logger->info("Found target '{}'", target.name->c_str());
    auto [consolidated, result] =
        constructConsolidatedTarget(cpakfile.targets, target);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    auto arguments = gatherCompilationArguments(consolidated);
    auto objects   = std::vector<std::string>();
    objects.reserve(target.sources->size());

    for (const auto& source : *target.sources) {
        const auto sourcePath = cpakfile.projectPath / source;
        const auto objectPath =
            cpakfile.objectsBuildPath() /
            fmt::format("{}.o", sourcePath.filename().c_str());

        logger->debug("Checking if source exists: {}", sourcePath.c_str());
        if (!std::filesystem::exists(sourcePath))
            return make_error_code(errc::pathDoesNotExist);

        // Add regardless of whether it exists or not, we're planning to build
        // it.
        logger->debug("Adding object: {}", objectPath.c_str());
        objects.emplace_back(objectPath.string());
        if (std::filesystem::exists(objectPath)) {
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
        std::string outputName;
        std::filesystem::path outputPath;

        switch (*consolidated.type) {
        case cpak::TargetType::Executable:
            outputPath = cpakfile.binariesBuildPath() / *consolidated.name;
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        case cpak::TargetType::StaticLibrary:
            outputName = fmt::format("lib{}.a", *consolidated.name);
            outputPath = cpakfile.binariesBuildPath() / outputName;
            arguments.emplace_back("-r");
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        case cpak::TargetType::DynamicLibrary:
            outputName = fmt::format("lib{}.so", *consolidated.name);
            outputPath = cpakfile.librariesBuildPath() / outputName;
            arguments.emplace_back("-shared");
            arguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));
            break;
        default: assert(false && "Invalid target type");
        }

        return executeInShell(arguments);
    });

    logger->debug("Queued for Linking: {}", target.name->c_str());
    return result;
}


std::error_code
cpak::queueForBuild(const cpak::CPakFile& cpakfile) noexcept {
    auto logger      = spdlog::get("cpak");
    auto queueStatus = make_error_code(errc::success);
    logger->info("Building project: {}", cpakfile.projectPath.string());

    const auto buildPath     = cpakfile.buildPath;
    const auto binariesPath  = buildPath / "binaries";
    const auto librariesPath = buildPath / "libraries";
    const auto objectsPath   = buildPath / "objects";
    if (!fs::exists(binariesPath)) fs::create_directories(binariesPath);
    if (!fs::exists(librariesPath)) fs::create_directories(librariesPath);
    if (!fs::exists(objectsPath)) fs::create_directories(objectsPath);

    for (const auto& dependency : cpakfile.dependencies) {
        const auto cpakid = identityToString(dependency);
        const auto iter   = dependencyCache.find(cpakid);
        if (iter == dependencyCache.end())
            return make_error_code(
                errc::failure); // Let the caller handle the error.

        queueStatus = queueForBuild(iter->second);
        if (queueStatus.value() != errc::success)
            return queueStatus; // Let the caller handle the error.
    }

    for (const auto& target : cpakfile.targets) {
        queueStatus = queueForBuild(cpakfile, target);
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
