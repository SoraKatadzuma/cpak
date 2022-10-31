import dataclasses
import yaml

from .dependency import Dependency
from .plugin     import Plugin
from .typedefs   import NullableList
from .utilities  import _validate_list



@dataclasses.dataclass
class Management(yaml.YAMLObject):
    yaml_tag    = "!Management"
    yaml_loader = yaml.SafeLoader

    plugins: NullableList[Plugin] = None
    depends: NullableList[Dependency] = None

    def __post_init__(self) -> None:
        self.plugins = _validate_list(self.plugins, Plugin)
        self.depends = _validate_list(self.depends, Dependency)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Management':
        return cls(**loader.construct_mapping(node, deep=True))


