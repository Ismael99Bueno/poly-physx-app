project "poly-physx-app"
staticruntime "off"
kind "StaticLib"

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
   "%{wks.location}/vendor/implot",
   "%{wks.location}/vendor/spdlog/include"
}
