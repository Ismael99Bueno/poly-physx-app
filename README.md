# poly-physx-app

poly-physx-app is an extension to the original [poly-physx](https://github.com/Ismael99Bueno/poly-physx) engine, providing visualization tools for the engine using SFML. It implements a base class for physics applications, featuring basic functionality such as visualizing entities, springs, rigid bars, camera movement, and more. poly-physx-app is designed to allow users to create their own applications using the engine and visualization tools as a foundation.

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
- [vec-2D](https://github.com/Ismael99Bueno/vec-2D): a 2D vector implementation with usual operations
- [shapes-2D](https://github.com/Ismael99Bueno/shapes-2D): a polygon geometry library for creating and manipulating convex polygons
- [rk-integrator](https://github.com/Ismael99Bueno/rk-integrator): an implementation of the Runge-Kutta method for integrating the movement of entities
- [debug-tools](https://github.com/Ismael99Bueno/debug-tools): a set of tools for debugging [poly-physx](https://github.com/Ismael99Bueno/poly-physx) simulations
- [profile-tools](https://github.com/Ismael99Bueno/profile-tools): a set of tools for profiling [poly-physx](https://github.com/Ismael99Bueno/poly-physx) simulations
- [ini-parser](https://github.com/Ismael99Bueno/ini-parser): a simple INI file parser that allows for reading and writing the state of the simulation to and from a file
- [vector-view](https://github.com/Ismael99Bueno/vector-view): A header only library for modifying the contents of a std::vector without letting the user to modify its size.

## Building and Usage

1. Build SFML from the [author's fork](https://github.com/Ismael99Bueno/SFML) using CMake. The SFML folder must be located in a `vendor` folder at the root of your workspace.
2. Reference the [SFML documentation](https://www.sfml-dev.org/tutorials/) for instructions on building the SFML library from source.
3. Build poly-physx-app using premake5 as a static library, with all other dependencies as premake5 projects.
4. Include all required dependencies as git submodules in your project.
5. Create a premake5 workspace with a user-implemented entry point that uses the poly-physx-app library.
6. Build the workspace with premake5 to create an executable.

For more information on how to use poly-physx-app, please refer to the documentation.

## License

poly-physx-app is licensed under the MIT License. See LICENSE for more information.
