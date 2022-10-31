import dataclasses
import yaml

from .typedefs import NullableData



@dataclasses.dataclass
class Remote(yaml.YAMLObject):
    yaml_tag    = "!Remote"
    yaml_loader = yaml.SafeLoader

    address:  str
    username: NullableData[str] = None
    password: NullableData[str] = None
    vcstool:  NullableData[str] = None
    branch:   NullableData[str] = None
    shallow:  bool = True

    @classmethod
    def from_yaml(cls, loader, node) -> 'Remote':
        return cls(**loader.construct_mapping(node, deep=True))
