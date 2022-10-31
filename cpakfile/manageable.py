import dataclasses
import yaml

from .identity  import Identity
from .remote    import Remote
from .typedefs  import NullableData
from .utilities import _validate_object



@dataclasses.dataclass
class Manageable(Identity):
    yaml_tag    = "!Manageable"
    yaml_loader = yaml.SafeLoader

    remote: NullableData[Remote] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.remote = _validate_object(self.remote, Remote)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Manageable':
        return cls(**loader.construct_mapping(node, deep=True))


