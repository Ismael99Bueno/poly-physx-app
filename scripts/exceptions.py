class PathNotFoundError(Exception):
    def __init__(self, path: str) -> None:
        self.__path = path
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"{self.__path} not found. The project's folder structure may have been changed"
