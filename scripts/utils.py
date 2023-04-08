import os
import subprocess
import shutil
from pathlib import Path
from exceptions import PathNotFoundError
import platform

ROOT_PATH = Path(os.path.dirname(os.path.realpath(__file__))).parent.absolute()


def clean_sfml() -> None:
    print("Removing build files for SFML...")
    build_sfml_path = f"{ROOT_PATH}/vendor/SFML/build-sfml"
    try:
        shutil.rmtree(build_sfml_path)
    except FileNotFoundError:
        print(
            f"Attempted to remove {build_sfml_path} directory, but it does not exist. Skipping..."
        )
    print("Done.\n")


def build_sfml() -> None:
    clean_sfml()
    print("Generating build files for SFML...")

    sfml_path = f"{ROOT_PATH}/vendor/SFML"
    if not os.path.exists(sfml_path):
        raise PathNotFoundError(sfml_path)

    build_sfml_path = f"{ROOT_PATH}/vendor/SFML/build-sfml"
    os.mkdir(build_sfml_path)

    mac_ver, _, arch = platform.mac_ver()
    subprocess.run(
        [
            "cmake",
            "-S",
            sfml_path,
            "-B",
            build_sfml_path,
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_OSX_ARCHITECTURES={arch}",
            f"-DCMAKE_OSX_DEPLOYMENT_TARGET={mac_ver.split('.')[0]}",
        ]
    )
    subprocess.run(["cmake", "--build", build_sfml_path])
    print("Done.\n")
