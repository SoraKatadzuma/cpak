import dataclasses
import enum
import yaml

from .platform  import Platform
from .typedefs  import NullableData
from .utilities import _validate_object

@enum.unique
class ActivationMode(enum.Enum):
    DEBUG   = 0
    STAGING = 1
    RELEASE = 2



@dataclasses.dataclass
class Activation(yaml.YAMLObject):
    yaml_tag    = "!Activation"
    yaml_loader = yaml.SafeLoader

    mode:     ActivationMode
    platform: NullableData[Platform] = None

    def __post_init__(self) -> None:
        if isinstance(self.mode, str):
            self.mode = ActivationMode[self.mode.upper()]
        self.platform = _validate_object(self.platform, Platform)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Activation':
        return cls(**loader.construct_mapping(node, deep=True))


