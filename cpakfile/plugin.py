import dataclasses
import yaml

from .manageable import Manageable
from .typedefs   import NullableData


@dataclasses.dataclass
class Plugin(Manageable):
    yaml_tag    = "!Plugin"
    yaml_loader = yaml.SafeLoader

    config: NullableData[dict] = None


    def __post_init__(self) -> None:
        super().__post_init__()


    @classmethod
    def from_yaml(cls, loader, node) -> 'Plugin':
        return cls(**loader.construct_mapping(node, deep=True))


