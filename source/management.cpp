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

    const auto& cpakfilePath = projectPath / "CPakFile";
    logger->info("Checking CPakfile '{}'", cpakfilePath.c_str());
    if (!std::filesystem::exists(cpakfilePath)) {
        loadStatus = make_error_code(errc::noCPakFileAtPath);
        return std::make_tuple(std::nullopt, loadStatus);
    }

    logger->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    std::optional<CPakFile> cpakfile;
    try {
        // Load CPakFile and set paths.
        cpakfile = YAML::LoadFile(cpakfilePath.string()).as<CPakFile>();
    } catch (const YAML::Exception& e) {
        // TODO: backtrace this.
        logger->error(fmt::format(fmt::fg(fmt::terminal_color::bright_red),
                                  "Failed to load CPakfile '{}'",
                                  cpakfilePath.c_str()));
        logger->error(fmt::format(fmt::fg(fmt::terminal_color::bright_red),
                                  "{}", e.what()));

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

    auto logger    = spdlog::get("cpak");
    auto remoteURL = dependency.remote != std::nullopt
                         ? *dependency.remote->address
                         : "https://github.com"s;

    logger->info("Cloning dependency '{}'", dependency.name->c_str());
    remoteURL += "/"s + *dependency.gpid + "/"s + *dependency.name;

    auto command = subprocess::Popen(
        { "git", "clone", remoteURL, dependencyPath },
        subprocess::output{ subprocess::PIPE },
        subprocess::error{ subprocess::PIPE }, subprocess::shell{ true });

    auto [output, error] = command.communicate();
    if (command.retcode() != errc::success)
        return std::make_tuple(std::nullopt,
                               make_error_code(errc::gitCloneFailed));

    logger->info("Cloned dependency '{}'", dependency.name->c_str());
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
        fmt::format("{}@{}", *dependency.name, dependency.semv->str());
    configPath = configPath / ".cpak" / *dependency.gpid / project;
    if (std::filesystem::exists(configPath))
        return std::make_tuple(configPath, make_error_code(errc::success));

    // Did not find dependency.
    return std::make_tuple(configPath, make_error_code(errc::pathDoesNotExist));
}