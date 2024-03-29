#include "errorcode.hpp"


struct CPakGenericErrorCategory : public std::error_category {
    const char*
    name() const noexcept override {
        return "cpak::generic_error";
    }

    std::string
    message(int code) const override {
        switch (code) {
        case cpak::errc::success: return cpak::errc::kSuccessMessage.data();
        case cpak::errc::failure: return cpak::errc::kFailureMessage.data();
        case cpak::errc::pathDoesNotExist:
            return cpak::errc::kPathDoesNotExistMessage.data();
        case cpak::errc::noCPakFileAtPath:
            return cpak::errc::kNoCPakFileAtPathMessage.data();
        case cpak::errc::invalidCPakFile:
            return cpak::errc::kInvalidCPakFileMessage.data();
        default: return "Unknown error";
        }
    }
};


struct CPakBuildErrorCategory : public std::error_category {
    const char*
    name() const noexcept override {
        return "cpak::build_error";
    }

    std::string
    message(int code) const override {
        // TODO: add build error codes.
        switch (code) {
        case cpak::errc::dependencyNotFound:
            return cpak::errc::kDependencyNotFoundMessage.data();
        case cpak::errc::gitRemoteNotFound:
            return cpak::errc::kGitRemoteNotFoundMessage.data();
        case cpak::errc::gitRemoteVersionNotFound:
            return cpak::errc::kGitRemoteVersionNotFoundMessage.data();
        case cpak::errc::gitCloneFailed:
            return cpak::errc::kGitCloneFailedMessage.data();
        case cpak::errc::noInstallInfo:
            return cpak::errc::kNoInstallInfoMessage.data();
        case cpak::errc::installTargetNotFound:
            return cpak::errc::kInstallTargetNotFoundMessage.data();
        case cpak::errc::libraryNotFound:
            return cpak::errc::kLibraryNotFoundMessage.data();
        case cpak::errc::interfaceNotFound:
            return cpak::errc::kInterfaceNotFoundMessage.data();
        case cpak::errc::interfaceNameCollision:
            return cpak::errc::kInterfaceNameCollisionMessage.data();
        default: return "Unknown error";
        }
    }
};


struct CPakUnknownErrorCategory : public std::error_category {
    const char*
    name() const noexcept override {
        return "cpak::unknown_error";
    }

    std::string
    message(int code) const override {
        return "Unknown error";
    }
};


const std::error_category&
cpak::generic_category() noexcept {
    static CPakGenericErrorCategory category;
    return category;
}


const std::error_category&
cpak::build_category() noexcept {
    static CPakBuildErrorCategory category;
    return category;
}


const std::error_category&
cpak::unknown_category() noexcept {
    static CPakUnknownErrorCategory category;
    return category;
}
