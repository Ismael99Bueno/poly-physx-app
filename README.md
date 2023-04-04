# poly-physx-app

poly-physx-app is an extension to the original [poly-physx](https://github.com/Ismael99Bueno/poly-physx) engine, providing visualization tools for it using SFML. It implements a base class for physics applications, featuring basic functionality such as visualizing entities, springs, rigid bars, camera movement, and more. poly-physx-app is designed to allow users to create their own applications using the engine and visualization tools as a foundation.

## Features

- Visualization tools for the [poly-physx](https://github.com/Ismael99Bueno/poly-physx) engine using SFML
- Base class for physics applications with essential functionality
- Ability to create custom applications with the engine and visual tools as a foundation
- Camera movement and visualization of entities, springs, and rigid bars
- UI integration with Dear ImGui using the imgui-sfml backend

## Dependencies

poly-physx-app depends on the following projects:

- [poly-physx](https://github.com/Ismael99Bueno/poly-physx): The original 2D physics engine
- [SFML](https://github.com/Ismael99Bueno/SFML): The Simple and Fast Multimedia Library (build from source using CMake from the fork on the author's GitHub page)
- [Dear ImGui](https://github.com/Ismael99Bueno/imgui): Immediate-mode graphical user interface library
- [imgui-sfml](https://github.com/Ismael99Bueno/imgui-sfml): ImGui backend for SFML
- [vec-2D](https://github.com/Ismael99Bueno/vec-2D): A 2D vector implementation with usual operations
- [shapes-2D](https://github.com/Ismael99Bueno/shapes-2D): A polygon geometry library for creating and manipulating convex polygons
- [rk-integrator](https://github.com/Ismael99Bueno/rk-integrator): An implementation of the Runge-Kutta method for integrating the movement of entities
- [debug-tools](https://github.com/Ismael99Bueno/debug-tools): A set of tools for debugging [poly-physx](https://github.com/Ismael99Bueno/poly-physx) simulations
- [profile-tools](https://github.com/Ismael99Bueno/profile-tools): A set of tools for profiling [poly-physx](https://github.com/Ismael99Bueno/poly-physx) simulations
- [ini-parser](https://github.com/Ismael99Bueno/ini-parser): A simple INI file parser that allows for reading and writing the state of the simulation to and from a file
- [vector-view](https://github.com/Ismael99Bueno/vector-view): A header only library for modifying the contents of a std::vector without letting the user to modify its size.
- [sfml-primitives](https://github.com/Ismael99Bueno/sfml-primitives): A C++ library providing line primitive implementations

There are two Python scripts located in the `scripts` folder: `fetch_dependencies.py` and `setup.py`. `fetch_dependencies.py` automatically adds all the dependencies as git submodules, provided that the user has already created their own repository and included the current project as a git submodule (or at least downloaded it into the repository). To ensure all runs smoothly once the script has been executed, do not rename the folders containing the various dependencies. All external dependencies, those not created by the same author, will be added as submodules within a folder called `vendor`.

Once all dependencies have been fetched using `fetch_dependencies.py`, execute the `setup.py` script to automatically build SFML from source as a shared library. Make sure to run `setup.py` after fetching all the dependencies to ensure a proper setup.


## Building and Usage

1. Ensure you have `premake5` and `make` installed on your system. `premake5` is used to generate the build files, and `make` is used to compile the project.
2. Create your own repository and include the current project as a git submodule (or at least download it into the repository).
3. Run the `fetch_dependencies.py` script located in the `scripts` folder to automatically add all the dependencies as git submodules.
4. Execute the `setup.py` script located in the `scripts` folder to automatically build [SFML](https://github.com/Ismael99Bueno/SFML) from source as a shared library.
5. Create an entry point project with a `premake5` file, where the `main.cpp` will be located. Link all libraries and specify the kind of the executable as `ConsoleApp`. Don't forget to specify the different configurations for the project.
6. Create a `premake5` file at the root of the repository describing the `premake` workspace and including all dependency projects.
7. Build the entire project by running the `make` command in your terminal. You can specify the configuration by using `make config=the_configuration`.
8. To use poly-physx-app, include the `app.hpp` header. Implement your custom app by inheriting from the `app` class.

For more information on how to use poly-physx-app, please refer to the documentation.

## License

poly-physx-app is licensed under the MIT License. See LICENSE for more information.
