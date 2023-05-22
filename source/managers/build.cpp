#include "build.hpp"
#include "../utilities/logging.hpp"
#include "../utilities/stropts.hpp"

using namespace cpak;

inline static void
copyInterfacePropertiesToTarget(const BuildTarget& interface,
                                      BuildTarget& result) noexcept {
    // This is a recursive function, so let's do this last.
    result.name       = interface.name;
    result.type       = interface.type;
    result.defines   |= interface.defines;
    result.libraries |= interface.libraries;
    result.sources   |= interface.sources;
    result.options   |= interface.options;

    // Copy search paths.
    if (result.search != std::nullopt) {
        result.search->include |= interface.search->include;
        result.search->system  |= interface.search->system;
        result.search->library |= interface.search->library;
    }
}

static std::error_code
flattenInterfaceTarget(const std::vector<BuildTarget>& targets,
                       const BuildTarget&              interface,
                             BuildTarget&              result) noexcept {
    // If this interface inherits from other interfaces, flatten those into this one.
    for (const auto& inherited : *interface.interfaces) {
        // Find the target in our list.
        const auto& targetInterface = std::find_if(
            targets.begin(), targets.end(),
            [&inherited](const auto& target) {
                return target.name == inherited;
            }
        );

        // If we didn't find the target report error.
        if (targetInterface == targets.end())
            return make_error_code(std::errc::invalid_argument);

        // If target is not an interfaces report error.
        if (targetInterface->type != TargetType::Interface)
            return make_error_code(std::errc::invalid_argument);

        // Flatten the interface.
        flattenInterfaceTarget(targets, *targetInterface, result);
    }

    // Copy properties from interface to target.
    copyInterfacePropertiesToTarget(interface, result);
    return std::error_code{ 0, std::generic_category() };
}

inline static std::error_code
constructConsolidatedTarget(const std::vector<BuildTarget>& targets,
                            const BuildTarget&              target,
                                  BuildTarget&              result) noexcept {
    return flattenInterfaceTarget(targets, target, result);
}


struct CommandResult {
    std::string output;
    std::string error;
    int         exitCode;
};

static inline CommandResult
executeInShell(const std::vector<std::string>& arguments) {
    // Build the source file.
    auto command = subprocess::Popen(
        arguments,
        subprocess::output{subprocess::STDOUT},
        subprocess::error{subprocess::STDERR},
        subprocess::shell{true}
    );

    auto result = command.communicate();
    return CommandResult {
        std::string(result.first.buf.begin(), result.first.buf.end()),
        std::string(result.second.buf.begin(), result.second.buf.end()),
        command.retcode()
    };
}



BuildManager::BuildManager(const std::shared_ptr<spdlog::logger>& logger)
    : logger_(logger)
{ }


void BuildManager::build(const std::optional<CPakFile>& project,
                         const std::filesystem::path&   projectPath,
                         const std::filesystem::path&   buildPath,
                               std::error_code&         buildStatus) const {
    using std::make_error_code;
    if (project == std::nullopt) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    // Collect all targets to build.
    logger_->info("Building project '{}'", "temp name");
    totalItemsToBuild_ = project->targets.size();
    for (const auto& target : project->targets)
        if (*target.type != TargetType::Interface)
            totalItemsToBuild_ += target.sources->size();

    // Assure build paths exist.
    const auto& binariesPath  = buildPath / "binaries";
    const auto& librariesPath = buildPath / "libraries";
    const auto& objectsPath   = buildPath / "objects";
    if (!std::filesystem::exists(buildPath))     std::filesystem::create_directories(buildPath);
    if (!std::filesystem::exists(binariesPath))  std::filesystem::create_directories(binariesPath);
    if (!std::filesystem::exists(librariesPath)) std::filesystem::create_directories(librariesPath);
    if (!std::filesystem::exists(objectsPath))   std::filesystem::create_directories(objectsPath);

    // Build all targets.
    for (const auto& target : project->targets) {
        // We do not build interfaces.
        if (target.type == TargetType::Interface)
            continue;

        // Consolidate a target with it's interfaces.
        BuildTarget consolidatedTarget;
        constructConsolidatedTarget(project->targets, target, consolidatedTarget);
        logger_->trace("\n{}", to_string(consolidatedTarget));

        // Build the target.
        buildTarget(*project, consolidatedTarget, projectPath, buildPath, buildStatus);
    }

    logger_->info("Finished building project '{}'", "temp name");
}

void BuildManager::resetBuildProgress() noexcept {
    totalItemsToBuild_ = 0;
    itemBuildProgress_ = 0;
    sourceErrorCount_  = 0;
}

void BuildManager::logBuildProgress(std::string_view message,
                                    std::string_view target) const noexcept {
    const auto& percentage = (float)itemBuildProgress_ /
                             (float)totalItemsToBuild_;

    logger_->info(
        "{:<4} {} '{}'",
        fmt::format("{}\%", std::round(percentage * 100.0f)),
        message.data(),
        target.data()
    );
}

inline void
BuildManager::logBuildCommand(const std::vector<std::string>& arguments) const noexcept {
    // Don't bother, this could be a very expensive operation.
    if (logger_->level() > spdlog::level::debug)
        return;

    // Log the command.
    std::ostringstream oss;
    for (const auto& arg : arguments)
        oss << arg << " ";
    logger_->debug(oss.str());    
}

std::vector<std::string>
BuildManager::getBuildArgumentsNoSources(const BuildTarget&     target,
                                               std::error_code& buildStatus) const {
    using std::begin, std::end, std::make_error_code;
    std::vector<std::string> arguments;

    // Reserve enough space for all arguments.
    auto size = target.defines->size() +
                target.libraries->size() +
                2; // g++ and options

    if (target.search != std::nullopt) {
        size += target.search->include->size();
        size += target.search->library->size();
        size += target.search->system->size();
    }

    arguments.reserve(size);
    arguments.emplace_back("g++");

    // Trim whitespaces from end of options if necessary.
    if (target.options != std::nullopt) {
        auto options = vectorPropertyToString(*target.options);
             options = utilities::trim(std::move(options));
        arguments.emplace_back(options);
    }

    // Add defines.
    utilities::reserveAndAppendFormatted(arguments, *target.defines, "-D {}");

    // Add search paths.
    if (target.search != std::nullopt) {
        utilities::reserveAndAppendFormatted(arguments, *target.search->include, "-I {}");
        utilities::reserveAndAppendFormatted(arguments, *target.search->library, "-L {}");
    }

    // Add linking libraries.
    utilities::reserveAndAppendFormatted(arguments, *target.libraries, "-l {}");
    return arguments;
}

void BuildManager::buildTarget(const CPakFile&              project,
                               const BuildTarget&           target,
                               const std::filesystem::path& projectPath,
                               const std::filesystem::path& buildPath,
                                     std::error_code&       buildStatus) const {
    using std::make_error_code;
    if (target.type == TargetType::Undefined) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    if (target.sources->empty()) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    buildSources(target, projectPath, buildPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to build sources for target '{}'", target.name->c_str());
        return;
    }

    itemBuildProgress_++;
    linkTarget(target, projectPath, buildPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to link target '{}'", target.name->c_str());
        return;
    }

    // Log source file being built.
    logBuildProgress("Built target", *target.name);
}

void BuildManager::buildSources(const BuildTarget&           target,
                                const std::filesystem::path& projectPath,
                                const std::filesystem::path& buildPath,
                                      std::error_code&       buildStatus) const {
    using std::make_error_code;
    const auto& arguments = getBuildArgumentsNoSources(target, buildStatus);
    for (const auto& source : *target.sources) {
        const auto& sourceFile = std::filesystem::path(source);
        const auto& sourcePath = projectPath / sourceFile;
        if (!std::filesystem::exists(sourcePath)) {
            buildStatus = make_error_code(std::errc::no_such_file_or_directory);
            return;
        }

        // Create output path.
        const auto& outputPath = buildPath  / "objects";
        const auto& outputFile = outputPath / fmt::format("{}.o", sourceFile.filename().c_str());
        
        // Proceed to compile sources.
        itemBuildProgress_++;
        compileSource(arguments, sourcePath, outputFile);
    }

    if (sourceErrorCount_ > 0) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }
}

void BuildManager::compileSource(const std::vector<std::string>& arguments,
                                 const std::filesystem::path&    sourcePath,
                                 const std::filesystem::path&    outputPath) const {
    using std::begin, std::end, std::make_error_code;

    // Reserve enough for the arguments, the source, and the output.
    std::vector<std::string> finalArguments;
    finalArguments.reserve(arguments.size() + 2);
    finalArguments.insert(end(finalArguments), begin(arguments), end(arguments));
    finalArguments.emplace_back(fmt::format("-c {}", sourcePath.c_str()));
    finalArguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));

    // Log source file being built.
    logBuildProgress("Compiling ", sourcePath.c_str());
    logBuildCommand(finalArguments);

    // Build the source file.
    const auto& result = executeInShell(finalArguments);
    if (result.exitCode == 0)
        return;

    // Print what happened.
    sourceErrorCount_++;
    logger_->error("\n{}", result.error);
    logger_->error("\n{}", result.output);
}

void BuildManager::linkTarget(const BuildTarget&           target,
                              const std::filesystem::path& projectPath,
                              const std::filesystem::path& buildPath,
                                    std::error_code&       buildStatus) const {
    using std::make_error_code;
    std::vector<std::string> arguments;
    arguments.reserve(target.sources->size() + 3); // sources, ld, c++ runtime, and output.
    arguments.emplace_back("g++");

    // Add sources.
    const auto& binariesPath  = buildPath / "binaries";
    const auto& librariesPath = buildPath / "libraries";
    const auto& objectsPath   = buildPath / "objects";
    for (const auto& source : *target.sources) {
        const auto& sourceFile = std::filesystem::path(source);
        const auto& sourcePath = objectsPath / fmt::format("{}.o", sourceFile.filename().c_str());
        arguments.emplace_back(sourcePath.c_str());
    }

    // Add library search directories.
    arguments.emplace_back(fmt::format("-L {}", binariesPath.c_str()));
    arguments.emplace_back(fmt::format("-L {}", librariesPath.c_str()));
    if (target.search.has_value())
        utilities::reserveAndAppendFormatted(arguments, *target.search->library, "-L {}");

    // Add linking libraries.
    utilities::reserveAndAppendFormatted(arguments, *target.libraries, "-l {}");

    // Add output type and output.
    std::string targetName;
    std::string outputName;
    std::filesystem::path targetPath;
    const auto& outputType = cpak::buildTypeName(*target.type);
    switch (*target.type) {
    case TargetType::Executable:
        targetPath = binariesPath / *target.name;
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    case TargetType::StaticLibrary:
        targetName = fmt::format("lib{}.a", *target.name);
        targetPath = librariesPath / targetName;
        arguments.emplace_back("-r");
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    case TargetType::DynamicLibrary:
        targetName = fmt::format("lib{}.so", *target.name);
        targetPath = binariesPath / targetName;
        arguments.emplace_back("-shared");
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    }

    // Log source file being built.
    logBuildProgress("Linking", targetPath.c_str());
    logBuildCommand(arguments);

    // Build the source file.
    const auto& result = executeInShell(arguments);
    if (result.exitCode == 0)
        return;

    // Print what happened.
    logger_->error("\n{}", result.error);
    logger_->error("\n{}", result.output);
}