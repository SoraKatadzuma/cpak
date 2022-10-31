import dataclasses
import pathlib
import yaml

from .build_target import BuildTarget
from .typedefs     import NullableData, NullableList, SerializableList
from .utilities    import _validate_list



@dataclasses.dataclass
class BuildInfo(yaml.YAMLObject):
    yaml_tag    = "!BuildInfo"
    yaml_loader = yaml.SafeLoader

    targets:   SerializableList[BuildTarget]
    threads:   NullableData[int] = None
    flags:     NullableData[str] = None
    includes:  NullableList[pathlib.Path] = None
    libraries: NullableList[pathlib.Path] = None

    def __post_init__(self) -> None:
        if (temp := _validate_list(self.targets, BuildTarget)) is None:
            raise ValueError("Targets cannot be None")
        self.targets = temp

        # HACK: this is an issue that should be resolved in the interpolator.
        if self.threads != None and isinstance(self.threads, str):
            self.threads = int(self.threads)

        self.includes = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'BuildInfo':
        return cls(**loader.construct_mapping(node, deep=True))
