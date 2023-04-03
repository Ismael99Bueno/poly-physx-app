import os
import subprocess
import shutil


def clean_sfml(root_path: str, /) -> None:
    print("Removing build files for SFML...")
    build_sfml_path = f"{root_path}/vendor/SFML/build-sfml"
    try:
        shutil.rmtree(build_sfml_path)
    except FileNotFoundError:
        print(
            f"Attempted to remove {build_sfml_path} directory, but it does not exist. Skipping..."
        )
    print("Done.\n")


def build_sfml(root_path: str, /) -> None:
    clean_sfml(root_path)
    print("Generating build files for SFML...")

    sfml_path = f"{root_path}/vendor/SFML"
    if not os.path.exists(sfml_path):
        raise FileNotFoundError(
            f"{sfml_path} not found. Did you pass the source path correctly?"
        )
    build_sfml_path = f"{root_path}/vendor/SFML/build-sfml"
    os.mkdir(build_sfml_path)
    subprocess.run(
        [
            "cmake",
            "-S",
            sfml_path,
            "-B",
            build_sfml_path,
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_OSX_ARCHITECTURES=arm64",
        ]
    )
    subprocess.run(["cmake", "--build", build_sfml_path])
    print("Done.\n")
