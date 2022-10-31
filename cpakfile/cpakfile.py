import dataclasses
import pathlib
import typing
import yaml

from .typedefs    import NullableData, NullableList, SerializableData
from .utilities   import PropertiesInterpolator, _validate_object, _validate_list
from .build_info  import BuildInfo
from .export_info import ExportInfo
from .management  import Management
from .parent      import ParentReference
from .profile     import Profile
from .project     import Project


def load_parent_cpakfile(parentref: ParentReference) -> 'CPakfile':
    # TODO: refactor this to use repo path from application.
    path = pathlib.Path.home() / f".cpak/repos/{parentref.name}"
    with path.open() as source:
        return yaml.safe_load(source)

    

@typing.final
@dataclasses.dataclass
class CPakfile(yaml.YAMLObject):
    yaml_tag    = "!CPakfile"
    yaml_loader = yaml.SafeLoader

    project: SerializableData[Project]
    build:   SerializableData[BuildInfo]

    parentref:  NullableData[ParentReference] = None
    management: NullableData[Management] = None
    export:     NullableData[ExportInfo] = None
    profiles:   NullableList[Profile] = None
    parent:     NullableData['CPakfile'] = dataclasses.field(default_factory=lambda: None, init=False)
    properties: dict = dataclasses.field(default_factory=lambda: {})


    def __post_init__(self) -> None:
        # Validate the parent reference and attempt to load the parents.
        if (temp := _validate_object(self.parentref, ParentReference)) is not None:
            assert isinstance(temp, ParentReference), \
                "ParentReference was not deserialized properly."
            self.parentref  = temp
            self.parent     = load_parent_cpakfile(temp)
            self.properties.update(self.parent.properties)
            self.properties.update({ "parent": self.__parent_properties() })

        if (temp := _validate_object(self.project, Project)) is None:
            raise ValueError("Project cannot be None")
        self.project = temp

        self.__interpolate_properties()
        if (temp := _validate_object(self.build, BuildInfo)) is None:
            raise ValueError("BuildInfo cannot be None")
        self.build = temp

        self.management = _validate_object(self.management, Management)
        self.export     = _validate_object(self.export, ExportInfo)
        self.profiles   = _validate_list(self.profiles, Profile)


    @classmethod
    def from_yaml(cls, loader, node) -> 'CPakfile':
        return cls(**loader.construct_mapping(node, deep=True))


    def __parent_properties(self) -> dict:
        exclude = ["properties"]
        keys    = self.parent.__dict__.keys()
        return { k: self.parent.__dict__[k] for k in keys - exclude }


    def __interpolate_properties(self) -> None:
        PropertiesInterpolator.properties(self.properties)
        PropertiesInterpolator.interpolate(self.build)      # type: ignore
        PropertiesInterpolator.interpolate(self.management) # type: ignore
        PropertiesInterpolator.interpolate(self.export)     # type: ignore
        PropertiesInterpolator.interpolate(self.profiles)   # type: ignore


