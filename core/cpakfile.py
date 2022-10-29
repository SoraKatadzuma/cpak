import dataclasses
import enum
import pathlib
import re
import typing
import yaml

from semantic_version import Version

from .typedefs   import SerializableData, SerializableList
from .exceptions import CPakError, NoParentModuleError
from .utils      import (
    PropertiesInterpolator,
    CPAKFILES,
    build_local_repo_path,
    _validate_object,
    _validate_list
)



@enum.unique
class BuildType(enum.Enum):
    HEADER     = 0
    LIBRARY    = 1
    ARCHIVE    = 2
    EXECUTABLE = 3



@enum.unique
class ActivationMode(enum.Enum):
    DEBUG   = 0
    STAGING = 1
    RELEASE = 2



@dataclasses.dataclass
class Author(yaml.YAMLObject):
    yaml_tag    = "!Author"
    yaml_loader = yaml.SafeLoader

    name:  str
    email: str

    @classmethod
    def from_yaml(cls, loader, node) -> 'Author':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Identity(yaml.YAMLObject):
    yaml_tag    = "!Identity"
    yaml_loader = yaml.SafeLoader

    name: str
    gpid: str
    semv: SerializableData[Version]

    def __post_init__(self) -> None:
        if (temp := _validate_object(self.semv, Version)) is None:
            raise ValueError("Version cannot be None")
        self.semv = temp

    @classmethod
    def from_yaml(cls, loader, node) -> 'Identity':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Project(Identity):
    yaml_tag    = "!Project"
    yaml_loader = yaml.SafeLoader

    desc:    typing.Optional[str] = None
    home:    typing.Optional[str] = None
    license: typing.Optional[str] = None
    authors: typing.Optional[SerializableList[Author]] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.authors = _validate_list(self.authors, Author)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Project':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Remote(yaml.YAMLObject):
    yaml_tag    = "!Remote"
    yaml_loader = yaml.SafeLoader

    address:  str
    username: str
    password: str
    vcstool:  typing.Optional[str] = None
    branch:   typing.Optional[str] = None
    shallow:  bool = True

    @classmethod
    def from_yaml(cls, loader, node) -> 'Remote':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Dependency(Identity):
    yaml_tag    = "!Dependency"
    yaml_loader = yaml.SafeLoader

    remote: typing.Optional[SerializableData[Remote]] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.remote = _validate_object(self.remote, Remote)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Dependency':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Plugin(Identity):
    yaml_tag    = "!Plugin"
    yaml_loader = yaml.SafeLoader

    config: typing.Optional[dict] = None

    @classmethod
    def from_yaml(cls, loader, node) -> 'Plugin':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Management(yaml.YAMLObject):
    yaml_tag    = "!Management"
    yaml_loader = yaml.SafeLoader

    plugins: typing.Optional[SerializableList[Plugin]] = None
    depends: typing.Optional[SerializableList[Dependency]] = None

    def __post_init__(self) -> None:
        self.plugins = _validate_list(self.plugins, Plugin)
        self.depends = _validate_list(self.depends, Dependency)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Management':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class BuildTarget(yaml.YAMLObject):
    yaml_tag    = "!BuildTarget"
    yaml_loader = yaml.SafeLoader

    name:      str
    type:      BuildType
    sources:   SerializableList[pathlib.Path]

    flags:     typing.Optional[str] = None
    includes:  typing.Optional[SerializableList[pathlib.Path]] = None
    libraries: typing.Optional[SerializableList[pathlib.Path]] = None

    def __post_init__(self) -> None:
        if isinstance(self.type, str):
            self.type = BuildType[self.type.upper()]
        if (temp := _validate_list(self.sources, pathlib.Path)) is None:
            raise ValueError("Sources cannot be none")
        self.sources = temp
        self.includes = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'BuildTarget':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class BuildInfo(yaml.YAMLObject):
    yaml_tag    = "!BuildInfo"
    yaml_loader = yaml.SafeLoader

    targets:   SerializableList[BuildTarget]
    threads:   typing.Optional[int] = None
    flags:     typing.Optional[str] = None
    includes:  typing.Optional[SerializableList[pathlib.Path]] = None
    libraries: typing.Optional[SerializableList[pathlib.Path]] = None

    def __post_init__(self) -> None:
        if (temp := _validate_list(self.targets, BuildTarget)) is None:
            raise ValueError("Targets cannot be None")
        self.targets = temp
        
        # HACK: this is an issue that should be resolved in the interpolator.
        if self.threads != None and isinstance(self.threads, str):
            self.threads = int(self.threads)

        self.includes = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'BuildInfo':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Platform(yaml.YAMLObject):
    yaml_tag    = "!Platform"
    yaml_loader = yaml.SafeLoader

    type: str
    dist: typing.Optional[str] = None
    arch: typing.Optional[str] = None
    semv: typing.Optional[SerializableData[Version]] = None

    def __post_init__(self) -> None:
        self.semv = _validate_object(self.semv, Version)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Platform':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Activation(yaml.YAMLObject):
    yaml_tag    = "!Activation"
    yaml_loader = yaml.SafeLoader

    mode:     ActivationMode
    platform: typing.Optional[SerializableData[Platform]] = None

    def __post_init__(self) -> None:
        if isinstance(self.mode, str):
            self.mode = ActivationMode[self.mode.upper()]
        self.platform = _validate_object(self.platform, Platform)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Activation':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class Profile(yaml.YAMLObject):
    yaml_tag    = "!Profile"
    yaml_loader = yaml.SafeLoader

    name:       str
    activation: SerializableData[Activation]
    properties: typing.Optional[dict] = None
    build:      typing.Optional[SerializableData[BuildInfo]]  = None
    management: typing.Optional[SerializableData[Management]] = None

    def __post_init__(self) -> None:
        if (temp := _validate_object(self.activation, Activation)) is None:
            raise ValueError("Activation cannot be None")
        self.activation = temp
        self.build      = _validate_object(self.build, BuildInfo)
        self.management = _validate_object(self.management, Management)

    @classmethod
    def from_yaml(cls, loader, node) -> 'Profile':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class ExportInfo(yaml.YAMLObject):
    yaml_tag    = "!ExportInfo"
    yaml_loader = yaml.SafeLoader

    includes:  typing.Optional[SerializableList[pathlib.Path]] = None
    libraries: typing.Optional[SerializableList[pathlib.Path]] = None

    def __post_init__(self) -> None:
        self.includes  = _validate_list(self.includes, pathlib.Path)
        self.libraries = _validate_list(self.libraries, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'ExportInfo':
        return cls(**loader.construct_mapping(node, deep=True))



@dataclasses.dataclass
class ParentReference(Identity):
    yaml_tag    = "!ParentReference"
    yaml_loader = yaml.SafeLoader

    path: typing.Optional[SerializableData[pathlib.Path]] = None

    def __post_init__(self) -> None:
        super().__post_init__()
        self.path = _validate_object(self.path, pathlib.Path)

    @classmethod
    def from_yaml(cls, loader, node) -> 'ParentReference':
        return cls(**loader.construct_mapping(node, deep=True))



def find_parent_config(pref: ParentReference) -> pathlib.Path:
    cfgdir = pathlib.Path.cwd()
    if pref.path != None:
        assert not isinstance(pref.path, dict)
        return cfgdir / pref.path

    cfgdir = build_local_repo_path(pref.name)
    if not cfgdir.exists():
        raise NoParentModuleError(pref)
    return cfgdir


def load_config(cfgdir: pathlib.Path) -> dict:
    ls = [x for x in cfgdir.iterdir()]
    for file in CPAKFILES:
        if file not in ls:
            continue
        config = cfgdir / file
        if config.is_dir():
            raise FileNotFoundError(f"CPakfile is directory {cfgdir}")
        with config.open() as source:
            return yaml.safe_load(source)
    raise FileNotFoundError(f"No CPakfile at {cfgdir}")



@typing.final
@dataclasses.dataclass
class CPakfile(yaml.YAMLObject):
    yaml_tag    = "!CPakfile"
    yaml_loader = yaml.SafeLoader

    project: SerializableData[Project]
    build:   SerializableData[BuildInfo]

    parent:     typing.Optional['CPakfile'] = dataclasses.field(default_factory=lambda: None, init=False)
    parentref:  typing.Optional[SerializableData[ParentReference]] = None
    management: typing.Optional[SerializableData[Management]] = None
    export:     typing.Optional[SerializableData[ExportInfo]] = None
    profiles:   typing.Optional[SerializableList[Profile]] = None
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


                
def load_parent_cpakfile(parentref: ParentReference) -> CPakfile:
    path = find_parent_config(parentref)
    with path.open() as source:
        return yaml.safe_load(source)
