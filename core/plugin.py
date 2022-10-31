import dataclasses
import pathlib
import typing


if typing.TYPE_CHECKING:
    from .application import Application


@dataclasses.dataclass
class PluginContext:
    app: Application



@typing.runtime_checkable
class Plugin(typing.Protocol):
    def initialize(self) -> None:
        raise NotImplementedError("Should be implemented by an external plugin")

    def terminate(self) -> None:
        raise NotImplementedError("Should be implemented by an external plugin")



@typing.final
class PluginManager:
    def __init__(self, app: Application) -> None:
        self.__context = PluginContext(app)

        return_path  = pathlib.Path.cwd()
        plugins_path = app.repository_path
