import dataclasses
import yaml

from semantic_version import Version
from .typedefs        import SerializableData
from .utilities       import _validate_object



@dataclasses.dataclass
class Identity(yaml.YAMLObject):
    yaml_tag    = "!Identity"
    yaml_loader = yaml.SafeLoader

    name: str
    gpid: str
    semv: SerializableData[Version]

    def __post_init__(self) -> None:
        if (temp := _validate_object(self.semv, Version)) is None:
            raise ValueError("Version cannot be None")
        self.semv = temp

    @classmethod
    def from_yaml(cls, loader, node) -> 'Identity':
        return cls(**loader.construct_mapping(node, deep=True))


