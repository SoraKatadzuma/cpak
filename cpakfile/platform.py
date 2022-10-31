import dataclasses
import yaml

from semantic_version import Version
from .typedefs        import NullableData
from .utilities       import _validate_object



@dataclasses.dataclass
class Platform(yaml.YAMLObject):
    yaml_tag    = "!Platform"
    yaml_loader = yaml.SafeLoader

    type: str
    dist: NullableData[str] = None
    arch: NullableData[str] = None
    semv: NullableData[Version] = None

    def __post_init__(self) -> None:
        self.semv = _validate_object(self.semv, Version)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Platform':
        return cls(**loader.construct_mapping(node, deep=True))


