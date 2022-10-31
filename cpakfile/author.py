import dataclasses
import yaml

@dataclasses.dataclass
class Author(yaml.YAMLObject):
    yaml_tag    = "!Author"
    yaml_loader = yaml.SafeLoader

    name:  str
    email: str

    @classmethod
    def from_yaml(cls, loader, node) -> 'Author':
        return cls(**loader.construct_mapping(node, deep=True))


