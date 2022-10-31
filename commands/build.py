import argparse
import cpakfile

from core.logging import logger
from .pull        import PullCommand


class BuildCommand:
    @classmethod
    def process(cls,
                projname: str,
                arguments: argparse.Namespace,
                management: cpakfile.Management,
                build_info: cpakfile.BuildInfo) -> None:
        assert isinstance(management.depends, list), \
            "management.depends is not of type list."

        assert isinstance(management.plugins, list), \
            "management.plugins is not of type list."

        assert isinstance(build_info.targets, list), \
            "Build info targets is missing or still serialized."

        # Pull deps and plugs if applicable.
        PullCommand.process(projname, arguments, management)

        # Start actual build sequence.
        logger.info(f"Initiated build for project '{cpakfile.project.name}'") # type: ignore
        # for target in build_info.targets:
        #     cls.__build_target()


