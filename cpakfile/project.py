import dataclasses
import enum
import yaml

from .author    import Author
from .identity  import Identity
from .typedefs  import NullableData, NullableList
from .utilities import _validate_list



@enum.unique
class ProjectType(enum.IntEnum):
    DEFAULT = 0
    BOM     = 1



@dataclasses.dataclass
class Project(Identity):
    yaml_tag    = "!Project"
    yaml_loader = yaml.SafeLoader

    type:    ProjectType = ProjectType.DEFAULT
    desc:    NullableData[str] = None
    home:    NullableData[str] = None
    license: NullableData[str] = None
    authors: NullableList[Author] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        if self.type == None:
            raise ValueError("Project Type cannot be None")
        if isinstance(self.type, str):
            self.type = ProjectType[self.type.upper()]
        self.authors = _validate_list(self.authors, Author)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Project':
        return cls(**loader.construct_mapping(node, deep=True))


