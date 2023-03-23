project "phys-app"
   kind "StaticLib"

   language "C++"
   cppdialect "C++20"
   staticruntime "on"

   targetdir("../bin/" .. outputdir)
   objdir("../build/" .. outputdir .. "/%{prj.name}")

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "/opt/homebrew/Cellar/sfml/2.5.1_2/include"}
