# poly-physx-app

poly-physx-app is an extension to the original [poly-physx](https://github.com/ismawno/poly-physx) engine, providing visualization tools for it using SFML. It implements a base class for physics applications, featuring basic functionality such as visualizing entities, springs, rigid bars, camera movement, and more. poly-physx-app is designed to allow users to create their own applications using the engine and visualization tools as a foundation.

## Features

- Visualization tools for the [poly-physx](https://github.com/ismawno/poly-physx) engine using SFML
- Base class for physics applications with essential functionality
- Ability to create custom applications with the engine and visual tools as a foundation
- Camera movement and visualization of entities, springs, and rigid bars
- UI integration with Dear ImGui using the imgui-sfml backend

## Dependencies

poly-physx-app depends on the following projects:

- [poly-physx](https://github.com/ismawno/poly-physx)
- [SFML](https://github.com/ismawno/SFML)
- [Dear ImGui](https://github.com/ismawno/imgui)
- [imgui-sfml](https://github.com/ismawno/imgui-sfml)
- [glm](https://github.com/g-truc/glm)
- [shapes-2D](https://github.com/ismawno/shapes-2D)
- [rk-integrator](https://github.com/ismawno/rk-integrator)
- [debug-tools](https://github.com/ismawno/debug-tools)
- [profile-tools](https://github.com/ismawno/profile-tools)
- [ini-parser](https://github.com/ismawno/ini-parser)
- [vector-view](https://github.com/ismawno/vector-view)
- [sfml-primitives](https://github.com/ismawno/sfml-primitives)

The script [fetch_dependencies.py](https://github.com/ismawno/poly-physx-app/scripts/fetch_dependencies.py) automatically adds all the dependencies as git submodules, provided that the user has already created their own repository and included the current project as a git submodule (or at least downloaded it into the repository). To ensure all runs smoothly once the script has been executed, do not rename the folders containing the various dependencies. All external dependencies, those not created by the same author, will be added as submodules within a folder called [vendor](https://github.com/ismawno/poly-physx-app/vendor).

Once all dependencies have been fetched using [fetch_dependencies.py](https://github.com/ismawno/poly-physx-app/scripts/fetch_dependencies.py), execute the [generate_build_files.py](https://github.com/ismawno/poly-physx-app/scripts/generate_build_files.py) script to automatically build SFML from source as a shared library. Make sure to run [generate_build_files.py](https://github.com/ismawno/poly-physx-app/scripts/generate_build_files.py) after fetching all the dependencies to ensure a proper setup.


## Building and Usage

1. Ensure you have `premake5` and `make` installed on your system. `premake5` is used to generate the build files, and `make` is used to compile the project.
2. Create your own repository and include the current project as a git submodule (or at least download it into the repository).
3. Run the [fetch_dependencies.py](https://github.com/ismawno/poly-physx-app/scripts/fetch_dependencies.py) script located in the [scripts](https://github.com/ismawno/poly-physx-app/scripts) folder to automatically add all the dependencies as git submodules.
4. Execute the [generate_build_files.py](https://github.com/ismawno/poly-physx-app/scripts/generate_build_files.py) script located in the [scripts](https://github.com/ismawno/poly-physx-app/scripts) folder to automatically build [SFML](https://github.com/ismawno/SFML) from source as a shared library.
5. Create an entry point project with a `premake5` file, where the `main.cpp` will be located. Link all libraries and specify the kind of the executable as `ConsoleApp`. Don't forget to specify the different configurations for the project.
6. Create a `premake5` file at the root of the repository describing the `premake` workspace and including all dependency projects.
7. Build the entire project by running the `make` command in your terminal. You can specify the configuration by using `make config=the_configuration`.
8. To use poly-physx-app, include the [app.hpp](https://github.com/ismawno/poly-physx-app/include/ppx/app.hpp) header. Implement your custom app by inheriting from the `app` class.

For more information on how to use poly-physx-app, please refer to the documentation.

## License

poly-physx-app is licensed under the MIT License. See LICENSE for more information.
