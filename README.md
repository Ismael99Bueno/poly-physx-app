# poly-physx-app

poly-physx-app is an extension to the original [poly-physx](https://github.com/ismawno/poly-physx) engine, providing visualization tools for it using [lynx](https://github.com/ismawno/lynx). It implements a base class for physics applications, featuring basic functionality such as visualizing bodies, joints, camera movement, and more. poly-physx-app is designed to allow me to easily create applications using the engine and visualization tools as a foundation.

## Features

- Visualization tools for the [poly-physx](https://github.com/ismawno/poly-physx) engine using [lynx](https://github.com/ismawno/lynx)
- Base class for physics applications with essential functionality
- Ability to create custom applications with the engine and visual tools as a foundation
- Camera movement and visualization of bodies and joints
- UI integration with Dear ImGui using the imgui-sfml backend

## Dependencies

poly-physx-app depends on the following projects:

- [poly-physx](https://github.com/ismawno/poly-physx)
- [geometry](https://github.com/ismawno/geometry)
- [rk-integrator](https://github.com/ismawno/rk-integrator)
- [cpp-kit](https://github.com/ismawno/cpp-kit)
- [yaml-cpp](https://github.com/ismawno/yaml-cpp) (optional)
- [spdlog](https://github.com/gabime/spdlog) (optional)
- [glfw](https://github.com/glfw/glfw)
- [imgui](https://github.com/ocornut/imgui)
- [implot](https://github.com/epezent/implot)

## Building and Usage

This project is intended to be used as a git submodule within another project (parent repo). A premake file is provided for building and linking poly-physx-app.

While these build instructions are minimal, this project is primarily for personal use. Although it has been built and tested on multiple machines (MacOS and Windows), it is not necessarily fully cross-platform or easy to build.

## License

poly-physx-app is licensed under the MIT License. See LICENSE for more information.
