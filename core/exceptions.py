class CPakError(Exception):
    pass


class NoParentModuleError(Exception):
    def __init__(self, parent, cause=None):
        self.__parent  = parent
        self.__message = f"No available parent module:\n{parent}"
        if cause != None:
            self.__message = self.__message + f"\n\tcaused by:{cause}"
        super().__init__(self.__message)

    @property
    def parent_reference(self):
        return self.__parent



class NoParentConfigurationError(Exception):
    def __init__(self, path: str):
        self.__path = path
        super().__init__(f"Parent ")

    def __str__(self):
        return

    @property
    def parent_path(self) -> str:
        return self.__path
