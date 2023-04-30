#include <algorithm>
#include <sstream>
#include "build.hpp"
#include "subprocess.hpp"
#include "spdlog/fmt/bundled/color.h"

static std::uint32_t totalItemsToBuild  = 0;
static std::uint32_t sourceErrorCount   = 0;
static std::uint32_t targetErrorCount   = 0;
static std::uint32_t targetSourcesBuilt = 0;
static std::uint32_t targetItemsBuilt   = 0;


static inline std::string rtrim(std::string str) {
    using std::rbegin, std::rend, std::end;
    str.erase(std::find_if(rbegin(str), rend(str),
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), end(str));
    return str;
}


cpak::BuildManager::BuildManager(const std::shared_ptr<spdlog::logger>& logger)
    : logger_(logger)
{ }


void cpak::BuildManager::build(const std::shared_ptr<cpak::CPakFile>& project,
                               const std::filesystem::path&           projectPath,
                                     std::error_code&                 buildStatus) const {
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
cpak::BuildManager::getBuildArgumentsNoSources(const cpak::BuildTarget& target,
                                                     std::error_code&   buildStatus) const {
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
        arguments.emplace_back(rtrim(*target.options));

    // Add defines.
    for (const auto& define : target.defines)
        arguments.emplace_back(fmt::format("-D{}", define));

    // Add search paths.
    if (target.search != std::nullopt) {
        for (const auto& includePath : target.search->include)
            arguments.emplace_back(fmt::format("-I {}", includePath));
        for (const auto& libraryPath : target.search->library)
            arguments.emplace_back(fmt::format("-L {}", libraryPath));
        for (const auto& systemPath : target.search->system)
            arguments.emplace_back(fmt::format("-isystem {}", systemPath));
    }

    // Add linking libraries.
    for (const auto& library : target.libraries)
        arguments.emplace_back(fmt::format("-l {}", library));

    return arguments;
}

void cpak::BuildManager::buildTarget(const cpak::BuildTarget&     target,
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

    switch (target.type) {
    case TargetType::Executable:
        return buildExecutable(target, projectPath, buildStatus);
    
    case TargetType::StaticLibrary: 
        return buildStaticLibrary(target, projectPath, buildStatus);
    
    case TargetType::DynamicLibrary:
        return buildDynamicLibrary(target, projectPath, buildStatus);
    }
}

void cpak::BuildManager::buildExecutable(const cpak::BuildTarget&     target,
                                         const std::filesystem::path& projectPath,
                                               std::error_code&       buildStatus) const {
    using std::make_error_code;
    logger_->info("Building executable '{}'", target.name.c_str());

    const auto& arguments = getBuildArgumentsNoSources(target, buildStatus);
    buildSources(target, projectPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to build sources for target '{}'", target.name.c_str());
        return;
    }

    linkExecutable(target, projectPath, buildStatus);
    if (buildStatus.value() != 0) {
        logger_->error("Failed to link target '{}'", target.name.c_str());
        return;
    }
}

void cpak::BuildManager::buildStaticLibrary(const cpak::BuildTarget&     target,
                                            const std::filesystem::path& projectPath,
                                                  std::error_code&       buildStatus) const {
    using std::make_error_code;
}

void cpak::BuildManager::buildDynamicLibrary(const cpak::BuildTarget&     target,
                                             const std::filesystem::path& projectPath,
                                                   std::error_code&       buildStatus) const {
    using std::make_error_code;
}

void cpak::BuildManager::buildSources(const cpak::BuildTarget&     target,
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
        outputPath = outputPath / fmt::format("{}.o", sourceFile.filename().c_str());
        compileSource(arguments, sourcePath, outputPath);
    }

    if (sourceErrorCount > 0) {
        buildStatus = make_error_code(std::errc::invalid_argument);
        return;
    }
}

void cpak::BuildManager::compileSource(const std::vector<std::string>& arguments,
                                       const std::filesystem::path&    sourcePath,
                                       const std::filesystem::path&    outputPath) const {
    using std::begin, std::end, std::make_error_code;

    // Reserve enough for the arguments, the source, and the output.
    std::vector<std::string> finalArguments;
    finalArguments.reserve(arguments.size() + 2);
    finalArguments.insert(end(finalArguments), begin(arguments), end(arguments));
    finalArguments.emplace_back(fmt::format("-c {}", sourcePath.c_str()));
    finalArguments.emplace_back(fmt::format("-o {}", outputPath.c_str()));

    // Log the command.
    std::ostringstream oss;
    for (const auto& arg : finalArguments)
        oss << arg << " ";
    logger_->warn(oss.str());

    // Build the source file.
    auto command = subprocess::Popen(
        finalArguments,
        subprocess::output{subprocess::STDOUT},
        subprocess::error{subprocess::STDERR},
        subprocess::shell{true}
    );

    auto result = command.communicate();
    if (command.retcode() == 0) {
        targetSourcesBuilt++;
        return;
    }

    sourceErrorCount++;
    const auto& coutString = std::string(result.first.buf.begin(), result.first.buf.end());
    const auto& cerrString = std::string(result.second.buf.begin(), result.second.buf.end());

    // TODO: backtrace errors.
    // Print what happened.
    logger_->error(fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Error compiling source file!"
    ));
    logger_->error(fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Result code: {}", command.retcode()
    ));
    logger_->error("{}\n{}\n{}", fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Error messages:"
    ), coutString, cerrString);
}

void cpak::BuildManager::linkExecutable(const cpak::BuildTarget&     target,
                                        const std::filesystem::path& projectPath,
                                              std::error_code&       buildStatus) const {
    using std::make_error_code;
    std::vector<std::string> arguments;
    arguments.reserve(target.sources.size() + 3); // sources, ld, c++ runtime, and output.
    arguments.emplace_back("g++");

    // Add sources.
    const auto& outputPath = projectPath / "build" / "objects";
    for (const auto& source : target.sources) {
        const auto& sourceFile = std::filesystem::path(source);
        const auto& sourcePath = outputPath / fmt::format("{}.o", sourceFile.filename().c_str());
        arguments.emplace_back(sourcePath.c_str());
    }

    // Add output.
    const auto& executable = projectPath / "build" / target.name;
    const auto& outputName = fmt::format("-o {}", executable.c_str());
    arguments.emplace_back(outputName);
    arguments.emplace_back("-lstdc++");

    // Log the command.
    std::ostringstream oss;
    for (const auto& arg : arguments)
        oss << arg << " ";
    logger_->warn(oss.str());

    // Build the executable.
    auto command = subprocess::Popen(
        arguments,
        subprocess::output{subprocess::STDOUT},
        subprocess::error{subprocess::STDERR},
        subprocess::shell{true}
    );

    auto result = command.communicate();
    if (command.retcode() == 0) {
        targetItemsBuilt++;
        return;
    }

    // TODO: track on a per-target basis.
    targetErrorCount++;
    const auto& coutString = std::string(result.first.buf.begin(), result.first.buf.end());
    const auto& cerrString = std::string(result.second.buf.begin(), result.second.buf.end());

    // TODO: backtrace errors.
    // Print what happened.
    logger_->error(fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Error linking executable!"
    ));
    logger_->error(fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Result code: {}", command.retcode()
    ));
    logger_->error("{}\n{}\n{}", fmt::format(
        fmt::fg(fmt::terminal_color::bright_red),
        "Error messages:"
    ), coutString, cerrString);
}