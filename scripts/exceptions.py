class PathNotFoundError(Exception):
    def __init__(self, path: str) -> None:
        self.__path = path
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"{self.__path} not found. If for some reason the root path is incorrect, try entering it manually with the --root-relpath option"
