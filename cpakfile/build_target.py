import dataclasses
import enum
import pathlib
import yaml

from .typedefs  import NullableData, NullableList, SerializableList
from .utilities import _validate_list



@enum.unique
class BuildToolChain(enum.IntEnum):
    GCC   = 0
    CLANG = 1
    MSVC  = 2



@enum.unique
class BuildType(enum.Enum):
    HEADER     = 0
    LIBRARY    = 1
    ARCHIVE    = 2
    EXECUTABLE = 3



@dataclasses.dataclass
class BuildTarget(yaml.YAMLObject):
    yaml_tag    = "!BuildTarget"
    yaml_loader = yaml.SafeLoader

    name:      str
    type:      BuildType
    toolchain: BuildToolChain
    sources:   SerializableList[pathlib.Path]

    flags:     NullableData[str] = None
    includes:  NullableList[pathlib.Path] = None
    libraries: NullableList[pathlib.Path] = None

    def __post_init__(self) -> None:
        if isinstance(self.type, str):
            self.type = BuildType[self.type.upper()]

        if self.toolchain == None:
            raise ValueError("Toolchain cannot be None")

        if isinstance(self.toolchain, str):
            self.toolchain = BuildToolChain[self.toolchain.upper()]

        if (temp := _validate_list(self.sources, pathlib.Path)) is None:
            raise ValueError("Sources cannot be none")

        self.sources = temp
        self.includes = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)


    @classmethod
    def from_yaml(cls, loader, node) -> 'BuildTarget':
        return cls(**loader.construct_mapping(node, deep=True))
