import pathlib
import re
import typing

from .typedefs import NullableData, NullableList, T

if typing.TYPE_CHECKING:
    from cpakfile import *


CPAKFILES = ["CPakfile", "CPakfile.yml", "CPakfile.yaml"]



def build_local_repo_path(name: str) -> pathlib.Path:
    # return cpakapp.config.repoloc / name
    pass




@typing.final
class PropertiesInterpolator:
    __properties: dict
    __expression: re.Pattern = re.compile(".*{.*}.*")

    @classmethod
    def properties(cls, props: dict) -> None:
        cls.__properties = props

    @classmethod
    def interpolate(cls, data: dict) -> None:
        for key in data:
            to_process = data[key]
            if isinstance(to_process, dict):
                cls.interpolate(to_process)
            elif isinstance(to_process, list):
                for item in to_process:
                    cls.interpolate(item)
            elif isinstance(to_process, str):
                cls.__try_interpolate_string(to_process, data)
            else:
                raise TypeError(to_process)

    @classmethod
    def __try_interpolate_string(cls, value: str, data: dict) -> bool:
        if value == None:
            return True

        # Needs to be able to load from hierarchy of the data.
        while cls.__expression.match(value) is not None:
            value = value.format(**data) # type: ignore
        return True



def _validate_object(data: NullableData[T], to_type: typing.Type[T]) -> NullableData[T]:
    if data == None:           return None
    if isinstance(data, dict): return to_type(**data)
    if isinstance(data, str):  return to_type(data)
    return data


def _validate_list(data: NullableList[T], to_type: typing.Type[T]) -> NullableList[T]:
    if data == None:
        return None

    result = []
    for item in data:
        result.append(_validate_object(item, to_type))
    return result
