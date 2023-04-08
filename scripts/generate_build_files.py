from argparse import ArgumentParser
import os
from utils import build_sfml, clean_sfml, ROOT_PATH
from exceptions import PathNotFoundError


def main() -> None:
    parser = ArgumentParser(
        description="Generate the build files for SFML.",
        epilog="Once the SFML build files have been generated, the project must have its own build files generated with premake, and compiled according to the chosen premake5 action.",
    )
    parser.add_argument(
        "--clean",
        dest="clean",
        action="store_const",
        const=True,
        default=False,
        help="clears all SFML build files",
    )

    args = parser.parse_args()

    if not os.path.exists(ROOT_PATH):
        raise PathNotFoundError(ROOT_PATH)

    print(f"Setup wrt root: {ROOT_PATH}\n")
    if args.clean:
        clean_sfml()
    else:
        build_sfml()


if __name__ == "__main__":
    main()
