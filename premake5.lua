project "poly-physx-app"
staticruntime "off"
kind "StaticLib"

language "C++"
cppdialect "c++20"

filter "system:macosx or linux"
   buildoptions {
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wconversion",
      "-Wno-unused-parameter",
      "-Wno-sign-conversion",
      "-Wno-gnu-anonymous-struct",
      "-Wno-nested-anon-types",
      "-Wno-string-conversion"
   }
filter {}

pchheader "ppx-app/internal/pch.hpp"
pchsource "src/internal/pch.cpp"

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
   "%{wks.location}/geometry/include",
   "%{wks.location}/rk-integrator/include",
   "%{wks.location}/cpp-kit/include",
   "%{wks.location}/vendor/yaml-cpp/include",
   "%{wks.location}/vendor/glfw/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui",
   "%{wks.location}/vendor/implot",
   "%{wks.location}/vendor/spdlog/include"
}
VULKAN_SDK = os.getenv("VULKAN_SDK")
filter "system:windows"
   includedirs "%{VULKAN_SDK}/Include"
