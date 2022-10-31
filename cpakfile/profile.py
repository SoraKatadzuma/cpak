import dataclasses
import yaml

from .activation import Activation
from .build_info import BuildInfo
from .management import Management
from .typedefs   import NullableData, SerializableData
from .utilities  import _validate_object



@dataclasses.dataclass
class Profile(yaml.YAMLObject):
    yaml_tag    = "!Profile"
    yaml_loader = yaml.SafeLoader

    name:       str
    activation: SerializableData[Activation]
    properties: NullableData[dict] = None
    build:      NullableData[BuildInfo] = None
    management: NullableData[Management] = None

    def __post_init__(self) -> None:
        if (temp := _validate_object(self.activation, Activation)) is None:
            raise ValueError("Activation cannot be None")
        self.activation = temp
        self.build      = _validate_object(self.build, BuildInfo)
        self.management = _validate_object(self.management, Management)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Profile':
        return cls(**loader.construct_mapping(node, deep=True))


