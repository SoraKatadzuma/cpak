import argparse
import dataclasses
import logging
import pathlib
import typing
import yaml

from application.arguments import parser
from cpakfile              import *
from cpakfile.utilities    import _validate_object
from commands.build        import BuildCommand
from commands.pull         import PullCommand

from enum       import IntEnum, unique
from .details   import LOGO_AND_VERSION
from .logging   import logger



DEFAULT_CPAK_PATH = pathlib.Path.home() / ".cpak"
DEFAULT_REPO_PATH = DEFAULT_CPAK_PATH / "repos"
DEFAULT_CONFIG_PATH = DEFAULT_CPAK_PATH / "config.yaml"



@unique
class SignalType(IntEnum):
    BUILD = 0



# TODO: Put into an function with other representers and execute before application load.
# To make sure paths get dumped correctly.
yaml.SafeDumper.add_multi_representer(
    pathlib.Path,
    lambda dumper, path:
        dumper.represent_str(str(path))
)


@typing.final
@dataclasses.dataclass
class Configuration(yaml.YAMLObject):
    yaml_tag    = "!CPakConfiguration"
    yaml_loader = yaml.SafeLoader
    yaml_dumper = yaml.SafeDumper

    repoloc: NullableData[pathlib.Path] = DEFAULT_REPO_PATH

    def __post_init__(self) -> None:
        self.repoloc = _validate_object(self.repoloc, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node):
        return cls(**loader.construct_mapping(node, deep=True))


    @classmethod
    def load_config(cls, cfgpath: pathlib.Path) -> 'Configuration':
        if not cfgpath.exists():
            return cls.build_config(cfgpath)

        result: 'Configuration'
        with cfgpath.open("r", encoding="utf-8") as source:
            result = yaml.safe_load(source)
            logger.debug(f"Loaded configuration '{str(cfgpath)}'")
        return result


    @classmethod
    def write_config(cls, cfgpath: pathlib.Path, config: 'Configuration') -> None:
        with cfgpath.open("w", encoding="utf-8") as file:
            yaml.safe_dump(config, file, indent=2, default_flow_style=False)
            logger.debug(f"Wrote configuration to file: {str(cfgpath)}")


    @classmethod
    def build_config(cls, cfgpath: pathlib.Path) -> 'Configuration':
        default = Configuration(repoloc=DEFAULT_REPO_PATH)
        cfgpath.parent.mkdir(parents=True, exist_ok=True)
        logger.warning(f"No CPak configuration, creating default.")
        cls.write_config(cfgpath, default)
        return default



@typing.final
class Application:
    def __init__(self) -> None:
        print("%s\n" % LOGO_AND_VERSION)
        arguments = parser.parse_args()
        if hasattr(arguments, "verbose"):
            logger.setLevel(logging.DEBUG)

        self.__config  = Configuration.load_config(DEFAULT_CONFIG_PATH)
        match arguments.action:
            case "build": self.__execute_build_action(arguments)
            case "clean":
                logger.info("Clean was requested")
            case "config":
                logger.info("Config was requested")
            case "install":
                logger.info("Install was requested")


    @property
    def repository_path(self) -> pathlib.Path:
        repo_path = pathlib.Path(DEFAULT_REPO_PATH)
        if self.__config.repoloc is not None:
            repo_path = self.__config.repoloc

        # Verifiably a path
        return repo_path # type: ignore


    def __execute_build_action(self, arguments: argparse.Namespace) -> None:
        cpakfile_path = pathlib.Path.cwd() / "CPakfile"
        if arguments.cpakfile is not None:
            cpakfile_path = pathlib.Path(arguments.cpakfile)

        logger.debug(f"Loading CPakfile '{str(cpakfile_path)}'")
        with cpakfile_path.open("r", encoding="utf-8") as source:
            cpakfile: CPakfile = yaml.safe_load(source)
            assert cpakfile.project is not None and \
                   isinstance(cpakfile.project, Project), \
                "cpakfile project is missing or still serialized."

            assert cpakfile.management is not None and \
                   isinstance(cpakfile.management, Management), \
                "cpakfile management is missing or still serialized."

            assert cpakfile.build is not None and \
                   isinstance(cpakfile.build, BuildInfo), \
                "cpakfile build info is missing or still serialized."

            assert cpakfile.project.name is not None, \
                "cpakfile project name is missing."

            projname = cpakfile.project.name
            BuildCommand.process(projname, arguments, cpakfile.management, cpakfile.build)


    def __execute_clean_action(self, arguments: argparse.Namespace) -> None:
        pass


    def __execute_config_action(self, arguments: argparse.Namespace) -> None:
        pass


    def __execute_install_action(self, arguments: argparse.Namespace) -> None:
        cpakfile_path = pathlib.Path.cwd() / "CPakfile"
        if arguments.cpakfile is not None:
            cpakfile_path = arguments.cpakfile



