import dataclasses
import yaml

from .author    import Author
from .identity  import Identity
from .typedefs  import NullableData, NullableList
from .utilities import _validate_list



@dataclasses.dataclass
class Project(Identity):
    yaml_tag    = "!Project"
    yaml_loader = yaml.SafeLoader

    desc:    NullableData[str] = None
    home:    NullableData[str] = None
    license: NullableData[str] = None
    authors: NullableList[Author] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.authors = _validate_list(self.authors, Author)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Project':
        return cls(**loader.construct_mapping(node, deep=True))


