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

filter "system:windows"
   defines "SFML_STATIC"
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
   "%{wks.location}/sfml-primitives/include",
   "%{wks.location}/vendor/glm",
   "%{wks.location}/vendor/imgui/include",
   "%{wks.location}/vendor/imgui-sfml/include",
   "%{wks.location}/vendor/SFML/include"
   "%{wks.location}/vendor/spdlog/include",
}
