import dataclasses
import yaml

from .manageable import Manageable



@dataclasses.dataclass
class Dependency(Manageable):
    yaml_tag    = "!Dependency"
    yaml_loader = yaml.SafeLoader


    def __post_init__(self) -> None:
        super().__post_init__()


    @classmethod
    def from_yaml(cls, loader, node) -> 'Dependency':
        return cls(**loader.construct_mapping(node, deep=True))


