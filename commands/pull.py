import argparse
import cpakfile

from core.logging  import logger



class PullCommand:
    @classmethod
    def process(cls,
                projname:   str,
                arguments:  argparse.Namespace,
                management: cpakfile.Management) -> None:
        assert isinstance(management.depends, list), \
            "management.depends is not of type list."

        assert isinstance(management.plugins, list), \
            "management.plugins is not of type list."

        logger.info(f"Initiated pull for project '{projname}'")

        # Exclusive arguments provided.
        if hasattr(arguments, "deps_only"):
            return cls.__pull_dependencies(management.depends)
        if hasattr(arguments, "plugs_only"):
            return cls.__pull_plugins(management.plugins)

        # Otherwise run both.
        cls.__pull_dependencies(management.depends)
        cls.__pull_plugins(management.plugins)


    @classmethod
    def __pull_dependencies(cls, dependencies: list) -> None:
        for dep in dependencies:
            assert isinstance(dep, cpakfile.Dependency), \
                "dep is not of type cpakfile.Dependency."
            cls.__try_download_manageable(dep)


    @classmethod
    def __pull_plugins(cls, plugins: list) -> None:
        for plug in plugins:
            assert isinstance(plug, cpakfile.Plugin), \
                "plug is not of type cpakfile.Plugin."
            cls.__try_download_manageable(plug)


    @classmethod
    def __try_download_manageable(cls, manageable: cpakfile.Manageable) -> None:
        # TODO: Check if repository is already available in repository site.
        # TODO: Check if repository is already up to date in repository site.
        # TODO: Download repository to repository site if not available or needs update.
        # TODO: If repository doesn't specify remote, throw exception and give up.
        pass

