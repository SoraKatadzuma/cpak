import pathlib
import re
import typing
import yaml

from .typedefs import NullableData, NullableList, T


CPAKFILES = ["CPakfile", "CPakfile.yml", "CPakfile.yaml"]


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
class PropertiesInterpolator:
    __properties: dict
    __expression: re.Pattern = re.compile(".*{.*}.*", re.DOTALL)

    @classmethod
    def properties(cls, props: dict) -> None:
        cls.__properties = props


    @classmethod
    def interpolate(cls, data: dict) -> None:
        cls.__try_interpolate(data)


    @classmethod
    def __try_interpolate(cls, data):
        if isinstance(data, int):
            return int(data)
        if isinstance(data, dict):
            return cls.__try_interpolate_dict(data)
        elif isinstance(data, list):
            return cls.__try_interpolate_list(data)
        elif isinstance(data, str):
            return cls.__try_interpolate_string(data)


    @classmethod
    def __try_interpolate_dict(cls, value: dict) -> dict:
        for key in value:
            value[key] = cls.__try_interpolate(value[key])
        return value


    @classmethod
    def __try_interpolate_list(cls, value: list) -> list:
        for idx in range(len(value)):
            value[idx] = cls.__try_interpolate(value[idx])
        return value


    @classmethod
    def __try_interpolate_string(cls, value: str) -> str:
        # Needs to be able to load from hierarchy of the data.
        while cls.__expression.match(value) is not None:
            value = value.format(**cls.__properties) # type: ignore
        return value



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
