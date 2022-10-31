import typing

T                = typing.TypeVar('T')
SerializableData = typing.Union[dict, str, T]
SerializableList = typing.List[SerializableData[T]]

NullableData = typing.Optional[SerializableData[T]]
NullableList = typing.Optional[SerializableList[T]]
