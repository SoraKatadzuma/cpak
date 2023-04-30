#include <algorithm>
#include <sstream>
#include "build.hpp"
#include "subprocess.hpp"
#include "spdlog/fmt/bundled/color.h"

using namespace cpak;

static std::uint32_t totalItemsToBuild = 0;
static std::uint32_t sourceErrorCount  = 0;
static std::uint32_t itemBuildProgress = 0;


static inline std::string ltrim(std::string str) {
    using std::begin, std::end;
    str.erase(begin(str), std::find_if(begin(str), end(str),
        std::not1(std::ptr_fun<int, int>(std::isspace))));
    return str;
}

static inline std::string rtrim(std::string str) {
    using std::rbegin, std::rend, std::end;
    str.erase(std::find_if(rbegin(str), rend(str),
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), end(str));
    return str;
}

static inline std::string trim(std::string str) {
    return ltrim(rtrim(std::move(str)));
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


void BuildManager::build(const std::shared_ptr<CPakFile>& project,
                         const std::filesystem::path&     projectPath,
                               std::error_code&           buildStatus) const {
    using std::make_error_code;
    if (project == nullptr) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    // Collect all targets to build.
    logger_->info("Building project '{}'", "temp name");
    totalItemsToBuild = project->targets.size();
    for (const auto& target : project->targets)
        totalItemsToBuild += target.sources.size();

    // Build all targets.
    for (const auto& target : project->targets) {
        // TODO: link targets.
        buildTarget(target, projectPath, buildStatus);
    }

    logger_->info("Finished building project '{}'", "temp name");
}

std::vector<std::string>
BuildManager::getBuildArgumentsNoSources(const BuildTarget&     target,
                                               std::error_code& buildStatus) const {
    using std::begin, std::end, std::make_error_code;
    std::vector<std::string> arguments;

    // Reserve enough space for all arguments.
    auto size = target.defines.size() +
                target.libraries.size() +
                2; // g++ and options

    if (target.search != std::nullopt) {
        size += target.search->include.size();
        size += target.search->library.size();
        size += target.search->system.size();
    }

    arguments.reserve(size);
    arguments.emplace_back("g++");

    // Trim whitespaces from end of options if necessary.
    if (target.options != std::nullopt)
        arguments.emplace_back(trim(*target.options));

    // Add defines.
    for (const auto& define : target.defines)
        arguments.emplace_back(fmt::format("-D{}", define));

    // Add search paths.
    if (target.search != std::nullopt) {
        for (const auto& includePath : target.search->include)
            arguments.emplace_back(fmt::format("-I {}", includePath));
        // Only really needed during linking.
        // for (const auto& libraryPath : target.search->library)
        //     arguments.emplace_back(fmt::format("-L {}", libraryPath));
        for (const auto& systemPath : target.search->system)
            arguments.emplace_back(fmt::format("-isystem {}", systemPath));
    }

    // Add linking libraries.
    for (const auto& library : target.libraries)
        arguments.emplace_back(fmt::format("-l {}", library));

    return arguments;
}

void BuildManager::buildTarget(const BuildTarget&           target,
                               const std::filesystem::path& projectPath,
                                     std::error_code&       buildStatus) const {
    using std::make_error_code;
    if (target.type == TargetType::Undefined) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    if (target.sources.empty()) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }

    buildSources(target, projectPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to build sources for target '{}'", target.name.c_str());
        return;
    }

    itemBuildProgress++;
    linkTarget(target, projectPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to link target '{}'", target.name.c_str());
        return;
    }

    // Log source file being built.
    const auto& percentage = (float)itemBuildProgress /
                             (float)totalItemsToBuild;

    // Because the length of the 100% string is 13.
    logger_->info(
        "{:<13} Built target '{}'",
        fmt::format(
            fmt::fg(fmt::terminal_color::bright_blue),
            "{}\%", std::round(percentage * 100.0f)
        ),
        target.name.c_str()
    );
}

void BuildManager::buildSources(const BuildTarget&           target,
                                const std::filesystem::path& projectPath,
                                      std::error_code&       buildStatus) const {
    using std::make_error_code;
    const auto& arguments = getBuildArgumentsNoSources(target, buildStatus);
    for (const auto& source : target.sources) {
        const auto& sourceFile = std::filesystem::path(source);
        const auto& sourcePath = projectPath / sourceFile;
        if (!std::filesystem::exists(sourcePath)) {
            buildStatus = make_error_code(std::errc::no_such_file_or_directory);
            return;
        }

        // Create the output directory if it doesn't exist.
        auto outputPath = projectPath / "build" / "objects";
        if (!std::filesystem::exists(outputPath))
            std::filesystem::create_directories(outputPath);

        // Proceed to compile sources.
        itemBuildProgress++;
        outputPath = outputPath / fmt::format("{}.o", sourceFile.filename().c_str());
        compileSource(arguments, sourcePath, outputPath);
    }

    if (sourceErrorCount > 0) {
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
    const auto& percentage = (float)itemBuildProgress /
                             (float)totalItemsToBuild;

    // Because the length of the 100% string is 13.
    logger_->info(
        "{:<13} Compiling source '{}'",
        fmt::format(
            fmt::fg(fmt::terminal_color::bright_blue),
            "{}\%", std::round(percentage * 100.0f)
        ),
        sourcePath.c_str()
    );

    // Log the command.
    std::ostringstream oss;
    for (const auto& arg : finalArguments)
        oss << arg << " ";
    logger_->debug(oss.str());

    // Build the source file.
    const auto& result = executeInShell(finalArguments);
    if (result.exitCode == 0)
        return;

    // Print what happened.
    sourceErrorCount++;
    logger_->error("\n{}", result.error);
    logger_->error("\n{}", result.output);
}

void BuildManager::linkTarget(const BuildTarget&           target,
                              const std::filesystem::path& projectPath,
                                    std::error_code&       buildStatus) const {
    using std::make_error_code;
    std::vector<std::string> arguments;
    arguments.reserve(target.sources.size() + 3); // sources, ld, c++ runtime, and output.
    arguments.emplace_back("g++");

    // Add sources.
    const auto& buildPath  = projectPath / "build";
    const auto& outputPath = buildPath / "objects";
    for (const auto& source : target.sources) {
        const auto& sourceFile = std::filesystem::path(source);
        const auto& sourcePath = outputPath / fmt::format("{}.o", sourceFile.filename().c_str());
        arguments.emplace_back(sourcePath.c_str());
    }

    // Add library search directories.
    arguments.emplace_back(fmt::format("-L {}", buildPath.c_str()));
    for (const auto& directory : target.search->library)
        arguments.emplace_back(fmt::format("-L {}", directory));

    // Add linking libraries.
    for (const auto& library : target.libraries)
        arguments.emplace_back(fmt::format("-l {}", library));

    // Add output type and output.
    std::string targetName;
    std::string outputName;
    std::filesystem::path targetPath;
    const auto& outputType = cpak::buildTypeName(target.type);
    switch (target.type) {
    case TargetType::Executable:
        targetPath = buildPath / target.name;
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    case TargetType::StaticLibrary:
        targetName = fmt::format("lib{}.a", target.name);
        targetPath = buildPath / targetName;
        arguments.emplace_back("-r");
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    case TargetType::DynamicLibrary:
        targetName = fmt::format("lib{}.so", target.name);
        targetPath = buildPath / targetName;
        arguments.emplace_back("-shared");
        arguments.emplace_back(fmt::format("-o {}", targetPath.c_str()));
        break;
    }

    // Log source file being built.
    const auto& percentage = (float)itemBuildProgress /
                             (float)totalItemsToBuild;

    // Because the length of the 100% string is 13.
    logger_->info(
        "{:<13} Linking {} '{}'",
        fmt::format(
            fmt::fg(fmt::terminal_color::bright_blue),
            "{}\%", std::round(percentage * 100.0f)
        ),
        outputType.c_str(),
        targetPath.c_str()
    );

    // Log the command.
    std::ostringstream oss;
    for (const auto& arg : arguments)
        oss << arg << " ";
    logger_->debug(oss.str());

    // Build the source file.
    const auto& result = executeInShell(arguments);
    if (result.exitCode == 0)
        return;

    // Print what happened.
    logger_->error("\n{}", result.error);
    logger_->error("\n{}", result.output);
}