import dataclasses
import pathlib
import yaml

from .typedefs  import NullableList
from .utilities import _validate_list



@dataclasses.dataclass
class ExportInfo(yaml.YAMLObject):
    yaml_tag    = "!ExportInfo"
    yaml_loader = yaml.SafeLoader

    includes:  NullableList[pathlib.Path] = None
    libraries: NullableList[pathlib.Path] = None

    def __post_init__(self) -> None:
        self.includes  = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'ExportInfo':
        return cls(**loader.construct_mapping(node, deep=True))


