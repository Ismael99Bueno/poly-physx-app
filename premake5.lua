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
         "shapes-2D",
         "glfw",
         "Cocoa.framework",
         "IOKit.framework",
         "CoreFoundation.framework",
         "vulkan",
         "imgui"
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
   "%{wks.location}/shapes-2D/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/debug-log-tools/include",
   "%{wks.location}/profile-tools/include",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/container-view/include",
   "%{wks.location}/allocators/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui",
   "%{wks.location}/vendor/spdlog/include"
}
