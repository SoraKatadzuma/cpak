import argparse

from cpakfile      import CPakfile, Project, Management
from core.logging  import logger
from .pull         import PullCommand


class BuildCommand:
    @classmethod
    def process(cls, arguments: argparse.Namespace, cpakfile: CPakfile) -> None:
        cls.__try_pull_command(arguments, cpakfile)
        logger.info(f"Initiated build for project '{cpakfile.project.name}'") # type: ignore


    @classmethod
    def __try_pull_command(cls, arguments: argparse.Namespace, cpakfile: CPakfile) -> None:
        assert cpakfile.project is not None and \
               isinstance(cpakfile.project, Project), \
            "cpakfile project is missing or still serialized."

        assert cpakfile.management is not None and \
               isinstance(cpakfile.management, Management), \
            "cpakfile management is missing or still serialized"

        assert cpakfile.project.name is not None, \
            "cpakfile project name is missing"

        PullCommand.process(cpakfile.project.name, arguments, cpakfile.management)


