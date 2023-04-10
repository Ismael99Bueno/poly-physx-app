project "poly-physx-app"
   staticruntime "off"
   kind "StaticLib"

   language "C++"
   cppdialect "C++17"
   buildoptions "-Wall"
   

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs "../**/include"
