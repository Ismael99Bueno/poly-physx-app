from argparse import ArgumentParser
import os
from utils import build_sfml, clean_sfml


def main() -> None:
    parser = ArgumentParser(description="Generate the build files for SFML.")
    parser.add_argument(
        "src",
        metavar="root-relpath",
        type=str,
        help="the relative path to the project root",
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

    root_path = os.path.abspath(args.src)
    if not os.path.exists(root_path):
        raise ValueError(f"Path {root_path} does not exist.")

    print(f"Setup wrt root: {root_path}\n")
    if args.clean:
        clean_sfml(root_path)
    else:
        build_sfml(root_path)


if __name__ == "__main__":
    main()
