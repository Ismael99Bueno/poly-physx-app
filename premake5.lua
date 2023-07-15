project "poly-physx-app"
staticruntime "off"
kind "ConsoleApp"

language "C++"
cppdialect "C++17"

filter "system:macosx"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter"
   }

   filter "kind:ConsoleApp"
      libdirs "%{wks.location}/vendor/vulkan-sdk/macOS/lib"
      links {
         "glfw",
         "Cocoa.framework",
         "IOKit.framework",
         "CoreFoundation.framework",
         "vulkan",
         "imgui",
         "cpp-kit",
         "yaml-cpp",
         "rk-integrator",
         "shapes-2D",
         "poly-physx",
         "lynx",
      }
      rpath = "-Wl,-rpath,".. rootpath .."vendor/vulkan-sdk/macOS/lib"
      linkoptions {rpath}
   filter {}
filter {}

pchheader "ppx-app/pch.hpp"
pchsource "src/pch.cpp"

targetdir("bin/" .. outputdir)
objdir("build/" .. outputdir)

files {
   "src/**.cpp",
   "include/**.hpp"
}
includedirs {
   "include",
   "%{wks.location}/poly-physx/include",
   "%{wks.location}/lynx/include",
   "%{wks.location}/shapes-2D/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/cpp-kit/include",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/vendor/glfw/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui",
   "%{wks.location}/vendor/spdlog/include"
}
