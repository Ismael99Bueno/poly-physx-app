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
   "../**/include",
   "../vendor/glm"
}
