project "poly-physx-app"
   staticruntime "off"
   kind "StaticLib"

   language "C++"
   cppdialect "C++17"
   filter "system:macosx"
      buildoptions {"-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Wno-unused-parameter"}
   filter "system:windows"
      defines "SFML_STATIC"
   filter {}
   

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "../vendor/glm"}
