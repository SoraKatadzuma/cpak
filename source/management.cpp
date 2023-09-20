#include "errorcode.hpp"
#include "management.hpp"
#include "option.hpp"
#include "target.hpp"
#include "utilities/checksum.hpp"
#include "utilities/stropts.hpp"


namespace fs   = std::filesystem;
namespace util = cpak::utilities;

using cpak::BuildOption;
using cpak::BuildTarget;
using cpak::CPakFile;
using std::string;
using std::vector;


extern void
updateOptions(CPakFile& cpakfile, const vector<string>& options) noexcept;

extern void
interpolateOptions(BuildTarget& target, const vector<BuildOption>& options);

extern void
interpolateOptions(CPakFile& cpakfile) noexcept;

extern std::tuple<std::optional<cpak::CPakFile>, std::error_code>
internalLoadCPakFile(const std::filesystem::path& projectPath) noexcept;


std::tuple<std::optional<cpak::CPakFile>, std::error_code>
cpak::management::loadCPakFile(
    const std::filesystem::path& projectPath) noexcept {
    // Assume success, until we find an error.
    auto loadStatus = make_error_code(errc::success);
    auto logger     = spdlog::get("cpak");

    logger->info("Checking path '{}'", projectPath.c_str());
    if (!std::filesystem::exists(projectPath)) {
        loadStatus = make_error_code(errc::pathDoesNotExist);
        return std::make_tuple(std::nullopt, loadStatus);
    }

    auto cpakfileFound = false;
    auto cpakfilePath  = std::filesystem::path();
    std::array<std::filesystem::path, 3> cpakfileVariants{
        projectPath/"CPakFile",
        projectPath/"CPakFile.yaml",
        projectPath/"CPakFile.yml"
    };
    
    for (const auto& path : cpakfileVariants) {
        logger->info("Searching for CPakfile '{}'", path.c_str());
        if (!std::filesystem::exists(path))
            continue;
        
        cpakfileFound = true;
        cpakfilePath = path;
        break;
    }

    // We exhausted our tries, so we can't find the CPakfile.
    if (!cpakfileFound) {
        loadStatus = make_error_code(errc::noCPakFileAtPath);
        return std::make_tuple(std::nullopt, loadStatus);
    }

    logger->info("Found CPakfile '{}'", cpakfilePath.c_str());
    logger->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    // Read file into string so we can use YAML::Marks to report errors.
    std::optional<CPakFile> cpakfile;
    std::ifstream cpakfileStream(cpakfilePath);
    if (!cpakfileStream.is_open()) {
        // TODO: replace this error with something more appropriate.
        loadStatus = make_error_code(errc::invalidCPakFile);
        return std::make_tuple(cpakfile, loadStatus);
    }
    
    std::string cpakfileAsString;
    std::stringstream temporaryStream;
    temporaryStream << cpakfileStream.rdbuf();
    cpakfileAsString = temporaryStream.str();
    cpakfileStream.close();

    try {
        // Load CPakFile and set paths.
        cpakfile = YAML::Load(temporaryStream.str()).as<CPakFile>();
    } catch (const YAML::Exception& e) {
        logger->error(fmt::format(fmt::fg(fmt::terminal_color::bright_red),
            "Failed to load CPakfile '{}'", cpakfilePath.c_str()));

        const auto lines = utilities::splitString(cpakfileAsString, "\n");

        // Reset and reuse.
        temporaryStream.clear();
        temporaryStream.str("");
        temporaryStream << "Error at line " << e.mark.line
                        << ", column " << e.mark.column
                        << " of " << cpakfilePath
                        << std::endl;
        
        // Extract Mark and add to error message.
        // Get 2-3 lines before error and line of error.
        const auto mark = e.mark;
        std::uint32_t longest = 0;
        for (auto count = 0u; count < 3; count++) {
            const auto start = mark.line - 1;
            const auto index = start < 0
                ? 0 + count
                : start + count;

            const auto line = lines[index];
            if (line.size() > longest)
                longest = line.size();
            
            temporaryStream << "  " << line << std::endl;
        }

        auto errorIndicator = std::string(e.msg.length() + mark.column, '_');
        auto errorExplicit  = std::string(mark.column, ' ');
        errorIndicator[mark.column] = '^';
        errorExplicit += e.msg;
        temporaryStream << "__" << errorIndicator << std::endl
                        << "  " << errorExplicit << std::endl;

        // TODO: backtrace this.
        logger->error(fmt::format(fmt::fg(fmt::terminal_color::bright_red),
            "{}", temporaryStream.str()));
        loadStatus = make_error_code(errc::invalidCPakFile);
        cpakfile   = std::nullopt;
    }

    return std::make_tuple(cpakfile, loadStatus);
}


std::tuple<std::optional<cpak::CPakFile>, std::error_code>
cpak::management::loadCPakFile(
    const std::filesystem::path& projectPath,
    const std::vector<std::string>& options) noexcept {
    // Will be modified later.
    auto loadStatus         = make_error_code(errc::success);
    auto [cpakfile, result] = loadCPakFile(projectPath);
    if (result.value() != errc::success)
        return std::make_tuple(cpakfile,
                               result); // Let the caller handle the error.

    // Update options, then interpolate them.
    ::updateOptions(*cpakfile, options);
    ::interpolateOptions(*cpakfile);

    // Update paths.
    cpakfile->projectPath = projectPath;
    cpakfile->buildPath   = projectPath / ".cpak" / ::util::checksum(*cpakfile);
    return std::make_tuple(cpakfile, result);
}


std::tuple<std::optional<cpak::CPakFile>, std::error_code>
cpak::management::loadDependency(const cpak::Dependency& dependency) noexcept {
    std::optional<CPakFile> cpakfile{ std::nullopt };

    // If the path isn't there, we need to clone the dependency.
    auto [path, result] = findDependencyPath(dependency);
    if (result.value() == cpak::errc::pathDoesNotExist)
        std::tie(cpakfile, result) = cloneDependency(dependency, path);
    if (result.value() == cpak::errc::success)
        std::tie(cpakfile, result) = internalLoadCPakFile(path);

    // return whatever we have, it's the caller's problem.
    return std::make_tuple(cpakfile, result);
}


std::tuple<std::optional<cpak::CPakFile>, std::error_code>
cpak::management::cloneDependency(const cpak::Dependency& dependency,
                                  const std::string& dependencyPath) noexcept {
    using namespace std::string_literals;

    // TODO: account for version.
    auto logger    = spdlog::get("cpak");
    auto remoteURL = dependency.remote != std::nullopt
                         ? dependency.remote->address
                         : "https://github.com"s;

    logger->info("Cloning dependency '{}'", dependency.name.c_str());
    remoteURL += "/"s + dependency.gpid + "/"s + dependency.name;

    // Check if remote has repo.
    logger->debug("Checking if remote '{}' exists", remoteURL.c_str());
    auto gitLsRemoteCommand = subprocess::Popen(
        { "git", "ls-remote", remoteURL },
        subprocess::output{ subprocess::PIPE },
        subprocess::error{ subprocess::PIPE }, subprocess::shell{ true });

    auto [output, error] = gitLsRemoteCommand.communicate();
    if (gitLsRemoteCommand.retcode() != errc::success) {
        logger->debug("Did not find remote '{}'", remoteURL.c_str());
        return std::make_tuple(std::nullopt,
                               make_error_code(errc::gitRemoteNotFound));
    }

    const auto version = dependency.versionIsBranch
        ? dependency.semv.prerelease()
        : dependency.semv.str();

    // Check if version/branch exists.
    logger->debug("Checking if version '{}' exists", version);
    const auto outputStr = std::string(output.buf.begin(), output.buf.end());
    if (outputStr.find(version) == std::string::npos) {
        logger->debug("Did not find version '{}'", version);
        return std::make_tuple(std::nullopt,
                               make_error_code(errc::gitRemoteVersionNotFound));
    }

    const auto versionFlag = "-b " + version;
    const auto depthFlag   = "--depth=1";

    // Try clone with version/branch flag.
    auto gitCloneCommand = subprocess::Popen(
        { "git", "clone", depthFlag, versionFlag, remoteURL, dependencyPath },
        subprocess::output{ subprocess::PIPE },
        subprocess::error{ subprocess::PIPE }, subprocess::shell{ true });

    std::tie(output, error) = gitCloneCommand.communicate();
    if (gitCloneCommand.retcode() != errc::success)
        return std::make_tuple(std::nullopt,
                               make_error_code(errc::gitCloneFailed));

    logger->info("Cloned dependency '{}'", dependency.name.c_str());
    return internalLoadCPakFile(dependencyPath);
}


std::tuple<std::filesystem::path, std::error_code>
cpak::management::findDependencyPath(
    const cpak::Dependency& dependency) noexcept {
#if _WIN32
    auto configPath = std::filesystem::path(std::getenv("USERPROFILE"));
#else
    auto configPath = std::filesystem::path(std::getenv("HOME"));
#endif

    const auto project =
        fmt::format("{}@{}", dependency.name, dependency.semv.str());
    configPath = configPath / ".cpak" / dependency.gpid / project;
    if (std::filesystem::exists(configPath))
        return std::make_tuple(configPath, make_error_code(errc::success));

    // Did not find dependency.
    return std::make_tuple(configPath, make_error_code(errc::pathDoesNotExist));
}