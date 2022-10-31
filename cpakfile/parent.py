import dataclasses
import pathlib
import yaml

from .identity  import Identity
from .typedefs  import NullableData
from .utilities import _validate_object



@dataclasses.dataclass
class ParentReference(Identity):
    yaml_tag    = "!ParentReference"
    yaml_loader = yaml.SafeLoader

    path: NullableData[pathlib.Path] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.path = _validate_object(self.path, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'ParentReference':
        return cls(**loader.construct_mapping(node, deep=True))


